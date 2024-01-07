#version 450 core

layout (binding = 0) uniform sampler2D amb_tex;
layout (binding = 1) uniform sampler2D emi_tex;
layout (binding = 2) uniform sampler2D dif_tex;
layout (binding = 3) uniform sampler2D sp_tex;
layout (binding = 4) uniform sampler2D sp_gl_tex;
layout (binding = 5) uniform sampler2D mr_tex;
layout (binding = 6) uniform sampler2D alb_tex;
layout (binding = 7) uniform sampler2D sp_dif_tex;
layout (binding = 8) uniform sampler2D tex_envmap;


in VS_OUT{
vec3 _normal;
vec3 _color;
vec2 _texCoords;
vec3 _view;
vec3 _position;
} fs_in;


out vec4 color;

uniform float ao;

#define LIGHTS 1
uniform vec3 lightPositions[LIGHTS] = {vec3(1.0, 1.0, 1.0)};
uniform vec3 lightColors[LIGHTS] = {vec3(1., 1., 1.)};
uniform vec3 camera;


const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}



void main()
{		
    vec3 albedo = pow(texture(alb_tex, fs_in._texCoords).rgb, vec3(2.2, 2.2, 2.2));
    float metallic = texture(mr_tex, fs_in._texCoords).g;
    float roughness = texture(mr_tex, fs_in._texCoords).r;
    float ao;

    vec3 N = normalize(fs_in._normal);
    vec3 V = normalize(camera - fs_in._position);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < LIGHTS; ++i) 
    {
        vec3 L = normalize(lightPositions[i] - fs_in._position);
        vec3 H = normalize(V + L);
        float distance    = length(lightPositions[i] - fs_in._position);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = lightColors[i] * attenuation;        
        
        float NDF = DistributionGGX(N, H, roughness);        
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kD = (vec3(1.0) - F)*(1.0 - metallic);        
        vec3 specular     = NDF * G * F / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);
            
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }   
  
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 col = ambient + Lo;
	
    col = col / (col + vec3(1.0));
    col = pow(col, vec3(1.0/2.2));  
   
    color = vec4(col, 1.0);
}