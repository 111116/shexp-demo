#version 410 core
in vec3 a_position;
in vec3 a_normal;
uniform mat4 u_view;
uniform mat4 u_projection;
out vec3 normal;
out vec3 position;

void main()
{
    gl_Position = u_projection * (u_view * vec4(a_position, 1.0));
    position = a_position;
    normal = a_normal;
}
