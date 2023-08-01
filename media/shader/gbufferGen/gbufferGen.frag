#version 330
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_in;

out vec4 FragColor;

layout (location = 0) out vec3 WorldPosOut; 
layout (location = 1) out vec3 DiffuseOut; 
layout (location = 2) out vec3 NormalOut; 
layout (location = 3) out vec3 TexCoordOut; 

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

uniform bool use_texture_kd;
uniform bool use_texture_normal;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

uniform Material material;

void main() 
{ 
    WorldPosOut = vs_in.FragPos; 
    if(use_texture_kd) DiffuseOut = texture(texture_diffuse1, vs_in.TexCoords).xyz; 
    else DiffuseOut = material.diffuse;
    if(use_texture_normal) NormalOut = texture(texture_normal1, vs_in.TexCoords).xyz; 
    else NormalOut = normalize(vs_in.Normal); 
    TexCoordOut = vec3(vs_in.TexCoords, 0.0); 
}

