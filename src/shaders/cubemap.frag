#version 330 core

// TODO 4-1
// Implement CubeMap shading

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, TexCoords);
}
