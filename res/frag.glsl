#version 410 core


#define PI (3.14159265359)

#define sh_order 3
#define N 9


in vec3 position; // world position (attribute interpolated)
in vec3 normal; // world normal (attribute interpolated)
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

void main()
{
// for all sphere blockers
// calculate angle & rotation
// accumulate log(occu)
    vec3 n = normalize(normal);
    // vec3 SHLightResult[N];

    float[N] f = float[N](0,0,0,0,0,0,0,0,0);

    // for (int i=0; i<30; ++i) {
    //     vec3 v = u_sphere[i].xyz - position;
    //     float dist = length(v);
    //     float angle = asin(u_sphere[i].w / dist);
    //     float[sh_order] cur_symmlog;
    //     for (int l=0; l<sh_order; ++l)
    //         cur_symmlog[l] = texture(u_log_lut, vec2((l+0.5)/sh_order, angle/(PI/2))).x;
    //     float[N] cur_log = rotate(cur_symmlog, normalize(v));
    //     for (int j=0; j<N; ++j)
    //         f[j] += cur_log[j];
    // }
    float[N] g = shexp(f);

    // for (int i=0; i<9; ++i)
       //  SHLightResult[i] = texture(u_LHcubemap, vec4(n,i)).rgb;
  //   logv;
  //   for (int i=0; i<nspheres; ++i) {
  //    vec3 distv = position - u_position[i];
  //    float dist = length(distv);
        // float angle = asin(u_radius[i] / dist);
        // t = texture()
     //    add(logv, rotate(t, normalize(distv));
  //   }

    // vec3 result = vec3(0.0);
    // for (int i = 0; i < 9; ++i)
    //     result += SHLightResult[i];

    // vec3 result = 3.5449075 * texture(u_LHcubemap, vec4(n,0)).rgb;
    vec3 LH[N];
    for (int i = 0; i < 9; ++i)
        LH[i] = texture(u_LHcubemap, vec4(n,i)).rgb;
    vec3 result = shdot(g, LH);
    // L_H dot product with SH_one, yields the integral of L_H

    result = 0.8 / 3.1416 * result; // times brdf
    
    // gamma correction
    float gamma = 2.2;
    result = max(result, 0.0);
    result = pow(result, vec3(1.0/gamma));
    o_color = vec4(result, 1.0);
}
