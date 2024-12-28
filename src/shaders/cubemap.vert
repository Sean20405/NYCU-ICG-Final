#version 330 core

// TODO 4-1
// Implement CubeMap shading

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(aPos, 0.005);
    gl_Position.w = gl_Position.z;

    TexCoords = aPos;
}