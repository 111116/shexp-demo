#version 410 core




// attributes
in vec3 a_position;
in vec3 a_normal;
in int a_clusterid;
// model transform
uniform mat4 u_view;
uniform mat4 u_projection;


out vec3 color; // linear color (radiance)
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
    gl_Position = u_projection * (u_view * vec4(a_position, 1.0));

    color = vec3(
    float(a_clusterid*123%256)/255,
    float(a_clusterid*777%256)/255,
    float(a_clusterid*817%256)/255);
}
