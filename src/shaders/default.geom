#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 TexCoord;
} gs_in[];

out vec2 TexCoord;
out vec4 ExplosionColor;
out float mixValue;

uniform float time;
uniform vec3 aExplosionColor;

vec4 explode(vec4 position, vec3 normal) {
    float magnitude = 10.0;
    vec3 direction = normal * pow(time, 0.3) * magnitude; 
    return position + vec4(direction, 0.0);
}

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}

void main() {
    if (time == 0.0) {  // no explosion
        for(int i = 0; i < 3; i++) {
            gl_Position = gl_in[i].gl_Position;
            TexCoord = gs_in[i].TexCoord;

            ExplosionColor = vec4(aExplosionColor, min(0.0, 1.0-time*2.0));
            mixValue = 0.0;

            EmitVertex();
        }
        EndPrimitive();
    }
    else {  // explosion
        vec3 normal = GetNormal();
        for(int i = 0; i < 3; i++) {
            gl_Position = explode(gl_in[i].gl_Position, normal);
            TexCoord = gs_in[i].TexCoord;

            ExplosionColor = vec4(aExplosionColor, min(0.0, 1.0 - time * 2.0));
            mixValue = min(1.0, time * 2.0);

            EmitVertex();
        }
        EndPrimitive();
    }
    
}