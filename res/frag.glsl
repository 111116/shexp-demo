#version 410 core


out vec4 o_color;
in vec3 color; // linear color, interpolated in linear space
in vec2 texcoord;
uniform sampler2D albedo_map;


void main()
{
    // gamma correction
    float gamma = 2.2;
    vec3 t = max(color, 0.0);
    t = pow(t, vec3(1.0/gamma));
    // objects without texture are marked with texcoord (-1,-1)
    if (texcoord != vec2(-1,-1))
    	t = vec3(1,1,0);
    	// t *= texture(albedo_map, texcoord).xyz;
    o_color = vec4(t, 1.0);
}
