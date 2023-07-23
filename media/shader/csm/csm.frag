#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform sampler2DArray shadowMap;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

uniform bool use_texture_kd;
uniform bool use_texture_ks;
uniform bool use_texture_normal;
uniform bool use_texture_height;

//fixed length, up to 16 cascade level
uniform mat4 lightSpaceMatrices[16];
uniform float cascadePlaneDistances[16];
uniform int cascadeCount;   // number of frusta - 1

uniform mat4 view;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

uniform Material material;

struct DirectionalLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float intensity;
};

uniform DirectionalLight dLight;

uniform vec3 viewPos; 

int layer;

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

float ShadowCalculation(vec4 fragPos, vec3 lightDir, vec3 normal)
{
    layer  = -1;
    vec4 fragPosViewSpace = view * fragPos;
    float depthViewSpace = abs(fragPosViewSpace.z);
    for(int i = 0; i < cascadeCount; i++){
        if(depthViewSpace < cascadePlaneDistances[i]){
            layer = i;
            break;
        }
    }
    if(layer == -1) layer = cascadeCount;

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * fragPos;

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, vec3(projCoords.xy,layer)).r; //d(blocker)
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z; //d(receiver)
    // calculate bias (based on depth map resolution and slope)
    //vec3 normal = normalize(fs_in.Normal);
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    // check whether current frag pos is in shadow
    //float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    //return shadow;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    // PCF PCSS
    bool use_PCF = true;    
    bool use_PCSS = true;
    int pcss_kernel_size = 5;
    int pcf_kernel_size = 5;
    int max_pcf_kernel_size = 15;
    if(use_PCF && use_PCSS){
        //in shadow
        float avg_blocker_distance = 0;
        int cnt = 0;
        for(int x = -pcss_kernel_size/2; x <= pcss_kernel_size/2; ++x)
        {
            for(int y = -pcss_kernel_size/2; y <= pcss_kernel_size/2; ++y)
            {
                float current_blocker_depth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r; 
                if(currentDepth - bias > current_blocker_depth){
                    avg_blocker_distance += current_blocker_depth;
                    cnt++;    
                }
     
            }    
        }
        if(cnt > 0){
            avg_blocker_distance /= cnt;
            pcf_kernel_size += int(pcf_kernel_size * (currentDepth - bias - avg_blocker_distance) / avg_blocker_distance); 
            if(pcf_kernel_size > max_pcf_kernel_size) pcf_kernel_size = max_pcf_kernel_size;
        }
        if(pcf_kernel_size % 2 == 0) pcf_kernel_size += 1;
    }
    // PCF
    if(use_PCF)
    {    
        for(int x = -pcf_kernel_size/2; x <= pcf_kernel_size/2; ++x)
        {
            for(int y = -pcf_kernel_size/2; y <= pcf_kernel_size/2; ++y)
            {
                float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r; 
                shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
            }    
        }
        shadow /= (pcf_kernel_size * pcf_kernel_size);
    }
    else
    {
        float depth = texture(shadowMap, vec3(projCoords.xy, layer)).r;
        shadow = currentDepth - bias > depth  ? 1.0 : 0.0;   
    }

    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;

}

void main()
{   
    // properties
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    
    //normal map
    if(use_texture_normal)
    {
        norm = texture(texture_normal1, fs_in.TexCoords).rgb;
        norm = normalize(norm * 2.0 - 1.0);   
        norm = normalize(fs_in.TBN * norm);
    }

    // directional lighting
    vec3 result = vec3(0.0f,0.0f,0.0f);
    result += CalcDirectionalLight(dLight, norm, fs_in.FragPos, viewDir);   
    FragColor = vec4(result,1.0);
}

// calculates the color when using a point light.
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.direction);
    //float shadow = 0.0;
    float shadow = ShadowCalculation(vec4(fragPos,1.0f), lightDir, normal); 
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
    // combine results
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    if(use_texture_kd)
    {
        diffuse = light.diffuse * diff * texture(texture_diffuse1,fs_in.TexCoords).rgb;
    }
    if(use_texture_ks)
    {
        specular = light.specular * spec * texture(texture_specular1,fs_in.TexCoords).rgb;
    }
    {
        //layer visulization
        //if(layer == 0) diffuse = vec3(1.0,0.0,0.0);
        //else if(layer == 1) diffuse = vec3(0.0,1.0,0.0);
        //else if(layer == 2) diffuse = vec3(0.0,0.0,1.0);
        //else if(layer == 3) diffuse = vec3(0.0,1.0,1.0);
        //else if(layer == 4) diffuse = vec3(1.0,1.0,0.0);
    }
    ambient *= light.intensity;
    diffuse *= light.intensity;
    specular *= light.intensity;

    return (ambient + (1.0 - shadow) * (diffuse + specular));
}