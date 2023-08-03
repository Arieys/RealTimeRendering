#version 330
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

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
    WorldPosOut = fs_in.FragPos; 
    if(use_texture_kd) DiffuseOut = texture(texture_diffuse1, fs_in.TexCoords).xyz; 
    else DiffuseOut = material.diffuse;
    if(use_texture_normal) {
        vec3 norm = texture(texture_normal1, fs_in.TexCoords).xyz;
        norm = normalize(norm * 2.0 - 1.0);   
        NormalOut = normalize(fs_in.TBN * norm);
    }
    else NormalOut = normalize(fs_in.Normal); 
    TexCoordOut = vec3(fs_in.TexCoords, 0.0); 
}

