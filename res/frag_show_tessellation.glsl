#version 410 core

in vec3 position; // world position (attribute interpolated)
in vec3 normal_interpolate; // world normal (attribute interpolated)
out vec4 o_color;
uniform vec3 u_coefficients[9]; // unused
uniform samplerCubeArray u_LHcubemap; // (dir, SHindex(l,m)) => L*H(dir), where L is environment light and H is cosine-weighted hemisphere
uniform samplerCubeArray u_sh_lut; // (dir, SHindex(l,m)) => evaluation of SH basis (l,m) at dir
uniform sampler2D u_log_lut; // (l, half angle) => V of sphere blocker, SH-projected, value at band l
uniform sampler2D u_ab_lut; // TODO, currently: (x, half angle) => coefficient, needs to be converted to be function of magnitude
uniform sampler2D u_sphere; // 1024x64 texture, used as array of vec4(position, radius)
uniform float max_magn;
uniform int n_sphere;

void main()
{
    o_color = vec4(
    float(gl_PrimitiveID*123%256)/255,
    float(gl_PrimitiveID*777%256)/255,
    float(gl_PrimitiveID*817%256)/255,
    1.0);
}
