#version 330 core
out vec4 FragColor;

in vec2 TexCoord; 

uniform sampler2D ourTexture;
uniform vec3 Color;
// uniform vec4 Color;
uniform bool noTexture;

void main()
{
    if(noTexture){
		FragColor = vec4(Color, 1.0);
		// FragColor = Color;
    }
    else{
        FragColor = texture(ourTexture, TexCoord);
    }
} 