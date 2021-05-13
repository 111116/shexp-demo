#version 410 core


#define PI (3.14159265359)

#define sh_order 15
#define N 225

// attributes
in vec3 a_position;
in vec3 a_normal;
in int a_objid;
in int a_clusterid;
in int a_sphcnt;
// model transform
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 color; // linear color (radiance)
uniform samplerCubeArray u_LHcubemap; // (dir, SHindex(l,m)) => L*H(dir), where L is environment light and H is cosine-weighted hemisphere
uniform samplerCubeArray u_sh_lut; // (dir, SHindex(l,m)) => evaluation of SH basis (l,m) at dir
uniform sampler2D u_log_lut; // (l, half angle) => V of sphere blocker, SH-projected, value at band l
uniform sampler2D u_ab_lut; // TODO, currently: (x, half angle) => coefficient, needs to be converted to be function of magnitude
uniform sampler2D u_sphere; // 1024x1024 texture of sphere (center, radius)
uniform sampler2DArray u_ratio; // sh_order x 1024x1024 texture of ratio
uniform sampler2D u_sparse;

uniform float max_magn;
uniform int gammasize;
uniform vec3 objcolor[10];



float[N] shmul(float[N] a, float[N] b)
{
    float[N] g = float[N](0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
    for (int i=0; i<gammasize; ++i)
    {
        vec4 lookup = texture(u_sparse, vec2((i%1024+0.5f)/1024, (i/1024+0.5f)/1024));
        int s_a = int(lookup.x);
        int s_b = int(lookup.y);
        int s_c = int(lookup.z);
        float s_val = lookup.w;
        g[s_c] += s_val * a[s_a] * b[s_b];
    }
    return g;
}

float[N] shsqr(float[N] f)
{
    // TODO optimize by symmetry
    return shmul(f,f);
}

float[N] shexp_naive_linear(float[N] f)
{
    // naive linear
    float[N] g;
    float e = exp(f[0]/sqrt(4.0/PI));
    g[0] = sqrt(4.0*PI)*e;
    for (int i = 1; i < N; i++)
    {
        g[i] = f[i]*e;
    }
    return g;
}

float[N] shexp_OL(float[N] f)
{
    // calculate magnitude of f_hat
    float magn = 0;
    for (int i=1; i<N; ++i)
        magn += f[i] * f[i];
    magn = sqrt(magn);
    // lookup optimal coefficients
    float u = (magn / max_magn * 1023 + 0.5) / 1024;
    float a = texture(u_ab_lut, vec2(0.0, u)).r;
    float b = texture(u_ab_lut, vec2(1.0, u)).r;
    float[N] g;
    // apply DC isolation
    float e = exp(f[0]/sqrt(4.0/PI));
    g[0] = a * sqrt(4.0*PI)*e;
    for (int i = 1; i < N; i++)
        g[i] = b * f[i]*e;
    return g;
}

float[N] shexp_HYB(float[N] f)
{
    // calculate magnitude of f_hat
    float magn = 0;
    for (int i=1; i<N; ++i)
        magn += f[i] * f[i];
    magn = sqrt(magn);
    // calculate times of scaling/squaring
    int p = 0;
    float k = 1;
    while (magn>0.25) {
        magn *= 0.5;
        k *= 0.5;
        p += 1;
    }
    // apply scaling/squaring
    for (int i=0; i<N; ++i)
        f[i] *= k;
    float[N] g = shexp_OL(f);
    for (int i=0; i<p; ++i)
        g = shsqr(g);
    return g;
}

float[N] shexp_PS9(float[N] f)
{
    // isolate DC component
    float e = exp(f[0]/sqrt(4.0/PI));
    f[0] = 0;
    // product series
    float[N] g = float[N](0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
    g[0] = sqrt(4.0*PI);
    float[N] a = f;
    for (int i = 1; i < 9; i++)
    {
        for (int j=0; j<N; ++j) {
            a[j] *= 1/i;
            g[i] += a[j];
        }
        a = shmul(a,f);
    }
    // apply DC isolation
    for (int j=0; j<N; ++j)
        g[j] *= e;
    return g;
}

float[N] shexp(float[N] f)
{
    return shexp_OL(f);
}

float[N] rotate(float[sh_order] a, vec3 w)
{
    float y[N];
    for (int j=0; j<N; ++j)
        y[j] = texture(u_sh_lut, vec4(w, j)).x;
    for (int l=0; l<sh_order; ++l) {
        float k = a[l] * sqrt(4*PI/(2*l+1));
        for (int m=-l; m<=l; ++m)
            y[l*l+l+m] *= k;
    }
    return y;
}

float shdot(float[N] a, float[N] b)
{
    float s = 0;
    for (int j=0; j<N; ++j)
        s += a[j] * b[j];
    return s;
}

vec3 shdot(float[N] a, vec3[N] b)
{
    vec3 s = vec3(0,0,0);
    for (int j=0; j<N; ++j)
        s += a[j] * b[j];
    return s;
}

float sqr(float a)
{
    return a*a;
}

vec4 texlookup(sampler2D tex, int i)
{
    int row = i/1024;
    int column = i%1024;
    return texture(tex, vec2((column+0.5f)/1024, (row+0.5f)/64));
}


void main()
{
    gl_Position = u_projection * (u_view * vec4(a_position, 1.0));

	// accumulate log(occu)
    float[N] f = float[N](0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	// for all sphere blockers
    for (int i=0; i<a_sphcnt; ++i) {
        // fetch i-th sphere
        vec4 sph_lookup = texture(u_sphere, vec2((i+0.5f)/1024, (a_clusterid+0.5f)/1024));
        vec3 center = sph_lookup.xyz;
        float radius = sph_lookup.w;
        vec3 v = center - a_position;
        float dist = length(v);
        // avoid self-shadowing
        float proj_dist = dot(v, a_normal);
        // eliminate spheres that are completely behind tangent surface
        // if (proj_dist + radius <= 0) {o_color = vec4(0,0.7,0,0); return;}
        if (proj_dist + radius <= 0) continue;
        // eliminate spheres covering p with centers behind tangent surface
        // if (dist < radius + 1e-5 && proj_dist <= 0) {o_color = vec4(0,0.7,0,0); return;}
        if (dist < radius + 1e-5 && proj_dist <= 0) continue;
        // for spheres that pokes through tangent surface:
        // make spheres not go behind tangent surface
        if (-radius <= proj_dist && proj_dist <= radius) {
            // if (dist < radius + 1e-5) {
            //     o_color = vec4(0,0,1,0); return;
            // }
            vec3 q0 = center + radius*a_normal;
            vec3 q1 = center - proj_dist*a_normal;
            v = (q0+q1)/2 - a_position;
            float d = sqrt(sqr(radius) - sqr(radius-length(q1-q0)));
            radius = (radius + proj_dist)/2;
            // apply a scaling factor to solve discontinuity
            if (proj_dist <= 0) {
                float alpha = min(1.0, (length(a_position-q1)-d)/d);
                if (alpha <= 0) continue;
                radius *= alpha;
                // o_color = vec4(1,0,0,0); return; // DEBUG
            }
        }
        float angle = asin(min(radius / dist,1));
        // look up log(visibility) of corresponding angle
        float[sh_order] cur_symmlog;
        for (int l=0; l<sh_order; ++l) {
        	float ratio = texture(u_ratio, vec3((i+0.5f)/1024, (a_clusterid+0.5f)/1024, l)).x;
            cur_symmlog[l] = ratio * texture(u_log_lut, vec2((l+0.5)/sh_order, angle/(PI/2))).x;
        }
        // rotate and accumulate
        float[N] cur_log = rotate(cur_symmlog, normalize(v));
        for (int j=0; j<N; ++j)
            f[j] += cur_log[j];
    }
    float[N] g = shexp(f);

    vec3 LH[N];
    for (int i = 0; i < N; ++i)
        LH[i] = texture(u_LHcubemap, vec4(a_normal.xzy,i)).rgb;
    vec3 result = shdot(g, LH);
    // L_H dot product with SH_one, yields the integral of L_H

    result = 1.0 / PI * objcolor[a_objid] * result; // times brdf
    color = result;
}
