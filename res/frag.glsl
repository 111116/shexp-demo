#version 410 core


out vec4 o_color;
in vec3 color; // linear color, interpolated in linear space


void main()
{
    // gamma correction
    float gamma = 2.2;
    vec3 t = max(color, 0.0);
    t = pow(t, vec3(1.0/gamma));
    o_color = vec4(t, 1.0);
}
