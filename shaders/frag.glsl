#version 410 core


out vec4 o_color;
in vec3 color; // linear color, interpolated in linear space
in vec2 texcoord;
flat in int objid;
uniform sampler2DArray albedo_map;


void main()
{
    // gamma correction
    float gamma = 2.2;
    vec3 t = max(color, 0.0);
    t = pow(t, vec3(1.0/gamma));
    // objects without texture are marked with texcoord (-1,-1)
    if (texcoord.x > -0.5 && texcoord.y > -0.5)
    	t *= texture(albedo_map, vec3(texcoord.x, 1-texcoord.y, objid)).xyz;
    o_color = vec4(t, 1.0);
}
