#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec4 ExplosionColor;
in float mixValue;

uniform sampler2D ourTexture;
uniform vec3 Color;
// uniform vec4 Color;
uniform bool noTexture;

void main()
{
    if(noTexture){
		FragColor = mix(vec4(Color, 1.0), ExplosionColor, mixValue);
		// FragColor = Color;
    }
    else{
        FragColor = mix(texture(ourTexture, TexCoord), ExplosionColor, mixValue);
    }
} 