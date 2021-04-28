#version 410 core


#define PI (3.14159265359)

#define sh_order 3
#define N 9


in vec3 position; // world position (attribute interpolated)
in vec3 normal_interpolate; // world normal (attribute interpolated)
out vec4 o_color;
uniform vec3 u_coefficients[9]; // unused
uniform samplerCubeArray u_LHcubemap; // (dir, SHindex(l,m)) => L*H(dir), where L is environment light and H is cosine-weighted hemisphere
uniform samplerCubeArray u_sh_lut; // (dir, SHindex(l,m)) => evaluation of SH basis (l,m) at dir
uniform sampler2D u_log_lut; // (l, half angle) => V of sphere blocker, SH-projected, value at band l
uniform sampler2D u_ab_lut; // TODO, currently: (x, half angle) => coefficient, needs to be converted to be function of magnitude
uniform vec4 u_sphere[30]; // vec4(position, radius)

// TODO: windowing

float[N] shexp(float[N] f)
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

void main()
{
// for all sphere blockers
// calculate angle & rotation
// accumulate log(occu)
    vec3 normal = normalize(normal_interpolate);
    // vec3 SHLightResult[N];

    float[N] f = float[N](0,0,0,0,0,0,0,0,0);

    for (int i=0; i<0; ++i) {
        vec3 center = u_sphere[i].xyz;
        float radius = u_sphere[i].w;
        vec3 v = center - position;
        float dist = length(v);
        // avoid self-shadowing
        float proj_dist = dot(v, normal);
        // eliminate spheres that are completely behind surface
        // if (proj_dist + radius <= 0) continue;
        // // eliminate spheres covering p with centers behind surface
        // if (dist < radius + 1e-5 && proj_dist <= 0) continue;
        // // make spheres not go behind tangent surface
        // if (-radius <= proj_dist && proj_dist <= radius) {
        //     vec3 q0 = center + radius*normal;
        //     vec3 q1 = center - proj_dist*normal;
        //     radius = (radius + proj_dist)/2;
        //     v = (q0+q1)/2 - position;
        //     float d = sqrt(sqr(radius) - sqr(radius-length(q1-q0)));
        //     float alpha = min(1.0, (length(position-q1)-d)/d);
        //     if (alpha <= 0) continue;
        //     radius *= alpha;
        // }
        float angle = asin(radius / dist);
        // look up log(visibility) of corresponding angle
        float[sh_order] cur_symmlog;
        for (int l=0; l<sh_order; ++l)
            cur_symmlog[l] = texture(u_log_lut, vec2((l+0.5)/sh_order, angle/(PI/2))).x;
        float[N] cur_log = rotate(cur_symmlog, normalize(v));
        for (int j=0; j<N; ++j)
            f[j] += cur_log[j];
    }
    float[N] g = shexp(f);

    vec3 LH[N];
    for (int i = 0; i < N; ++i)
        LH[i] = texture(u_LHcubemap, vec4(normal.xzy,i)).rgb;
    vec3 result = shdot(g, LH);
    // L_H dot product with SH_one, yields the integral of L_H

    result = 0.8 / 3.1416 * result; // times brdf
    
    // gamma correction
    float gamma = 2.2;
    result = max(result, 0.0);
    result = pow(result, vec3(1.0/gamma));
    o_color = vec4(result, 1.0);
}
