 #version 330 
layout (location = 0) in vec3 Position; 
layout (location = 1) in vec2 TexCoord; 
layout (location = 2) in vec3 Normal; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

void main()
{ 
    gl_Position = projection * view * model * vec4(Position, 1.0);
    vs_out.TexCoords = TexCoord; 
    vs_out.Normal = (model * vec4(Normal, 0.0)).xyz; 
    vs_out.FragPos = (model * vec4(Position, 1.0)).xyz;
}

