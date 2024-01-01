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

const float PI = 3.14159265359f;




uniform bool isTexture = true;
uniform vec3 camera;
layout (binding = 0, rg16f) uniform readonly image2D image;

uniform int NUM_STEPS_INT = 15;
uniform float SPECULAR_FACTOR = 16.0f;
uniform float G = 0.f;

uint width = 1900;
uint height = 1000;
vec2 size = vec2(1, 1);
float stepSize = (imageLoad(image, ivec2(size*gl_FragCoord.xy)).y - imageLoad(image, ivec2(size*gl_FragCoord.xy)).x)/NUM_STEPS_INT;

in VS_OUT {
vec3 _normal;
vec3 _color;
vec2 _texCoor;
vec3 _view;
vec3 _position;
}fs_in;

float shininess = 1.;

out vec4 color;

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS] = {{vec3(1.5, 1.5, 1.5), 0.1, 0.5, 0.5, vec3(1., 1., 1.), vec3(1., 1., 1.), vec3(1., 1., 1.)}};



float CalcScattering(float cosTheta, float G)
{
    return (1.0f - G * G) / (4.0f * PI * pow(1.0f + G * G - 2.0f * G * cosTheta, 1.5f));
}

vec3 CalcVolumeScattering(vec3 viewDir, vec3 lightDir, float G, vec3 color)
{
    vec3 step = -viewDir * stepSize; // viewDir or -viewDir
 
    vec3 position = camera;
    vec3 volumetric = vec3(0.0f);
    for (int i = 0; i < NUM_STEPS_INT; ++i)
    {
        volumetric += CalcScattering(dot(viewDir, lightDir), G) * color; //color
        position += step;
    }
    volumetric /= float(NUM_STEPS_INT);
    return volumetric;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient  = light.ambient  * vec3(texture(alb_tex, fs_in._texCoor));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(sp_dif_tex, fs_in._texCoor));
    vec3 specular = light.specular * spec * vec3(texture(mr_tex, fs_in._texCoor));

    vec3 volumetric = CalcVolumeScattering(viewDir, lightDir, G, light.diffuse);
    float density = 1.f;
    float inscattering = exp(0.f);
    volumetric *= density * inscattering;

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular + volumetric);
}



void main()
{
    vec3 norm = normalize(fs_in._normal);
    vec3 viewDir = normalize(camera - fs_in._position);
    vec3 result = vec3(0., 0., 0.);
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, fs_in._position, viewDir);

    //color = vec4(result, 1.0);
    float a = imageLoad(image, ivec2(size.xy*gl_FragCoord.xy)).y;
    color = vec4(a,a,a,1.0f);
}
