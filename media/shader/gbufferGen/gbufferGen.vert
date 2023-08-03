 #version 330 
layout (location = 0) in vec3 Position; 
layout (location = 1) in vec2 TexCoord; 
layout (location = 2) in vec3 Normal; 
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;

void main()
{ 
    gl_Position = projection * view * model * vec4(Position, 1.0);
    vs_out.TexCoords = TexCoord; 
    vs_out.Normal = (model * vec4(Normal, 0.0)).xyz; 
    vs_out.FragPos = (model * vec4(Position, 1.0)).xyz;

    //calculate TBN matrix
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * Normal);
    vec3 T = normalize(normalMatrix * tangent);
    vec3 B = normalize(normalMatrix * bitangent);
    vs_out.TBN = mat3(T, B, N);
}

