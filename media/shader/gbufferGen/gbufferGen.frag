#version 330
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_in;

layout (location = 0) out vec3 WorldPosOut; 
layout (location = 1) out vec3 DiffuseOut; 
layout (location = 2) out vec3 NormalOut; 
layout (location = 3) out vec3 TexCoordOut; 

uniform sampler2D gColorMap; 

void main() 
{ 
    WorldPosOut = vs_in.FragPos; 
    //DiffuseOut = texture(gColorMap, vs_in.TexCoords).xyz; 
    DiffuseOut = vec3(0.0,1.0,1.0); 
    NormalOut = normalize(vs_in.Normal); 
    TexCoordOut = vec3(vs_in.TexCoords, 0.0); 
}

