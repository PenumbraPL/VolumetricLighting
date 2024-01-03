#version 450 core
//#extension GL_NV_shader_buffer_load : enable
/*
layout (binding = 0) uniform sampler2D amb_tex;
layout (binding = 1) uniform sampler2D emi_tex;
layout (binding = 2) uniform sampler2D dif_tex;
layout (binding = 3) uniform sampler2D sp_tex;
layout (binding = 4) uniform sampler2D sp_gl_tex;
layout (binding = 5) uniform sampler2D mr_tex;
layout (binding = 6) uniform sampler2D alb_tex;
layout (binding = 7) uniform sampler2D sp_dif_tex;
layout (binding = 8) uniform sampler2D tex_envmap;
*/
uniform bool isTexture = true;
uniform vec3 camera;


const float PI = 3.14159265359f;
uniform int NUM_STEPS_INT = 15;
uniform float SPECULAR_FACTOR = 16.0f;
uniform float G = -0.8f;


float stepSize = (gl_FragCoord.z - 0.001f) / NUM_STEPS_INT;

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

// offset
layout (binding = 0, std430) buffer lights{
    uint size;
    PointLight list[];
};


float CalcScattering(float cosTheta, float G)
{
    return 3.f * (1.f - G * G) * (1.f + cosTheta * cosTheta) / (8 * PI * (2 + G * G) * pow(1 + G * G - 2 * G * cosTheta, 1.5));
}

vec3 CalcVolumeScattering(vec3 viewDir, PointLight light, float G, vec3 color, vec3 fragPos)
{
    vec3 step = -viewDir * stepSize; // viewDir or -viewDir
 
    vec3 position = fragPos;//camera;
    vec3 volumetric = vec3(0.0f);
    for (int i = 0; i < NUM_STEPS_INT; ++i)
    {
        vec3 lightDir = normalize(light.position - position);
        volumetric += CalcScattering(dot(lightDir, viewDir), G) * color;
        position += step;
    }
   // volumetric /= float(NUM_STEPS_INT);
    return volumetric;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient  = light.ambient  * 0.5;//vec3(texture(alb_tex, fs_in._texCoor));
    vec3 diffuse  = light.diffuse  * diff * 0.5;//vec3(texture(sp_dif_tex, fs_in._texCoor));
    vec3 specular = light.specular * spec * 0.5;//vec3(texture(mr_tex, fs_in._texCoor));

    vec3 volumetric = CalcVolumeScattering(viewDir, light, G, vec3(1.f, 1.f, 1.f), fragPos);
    float density = 1.f;
    float inscattering = exp(0.f);
    volumetric *= density * inscattering;

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));
    attenuation *= 0;
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
    for(int i = 0; i < size; i++)
        result += CalcPointLight(list[i], norm, fs_in._position, viewDir);

    color = vec4(vec3(1.f, 1.f, 1.f), result.x);
}
