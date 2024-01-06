#version 450 core

uniform vec3 camera;
uniform int NUM_STEPS_INT = 20;
uniform float SPECULAR_FACTOR = 16.0f;
uniform float G = -0.8f;
uniform vec3 bb_min = vec3(-2., -2., -2.);
uniform vec3 bb_max = vec3(2., 2., 2.);
//uniform vec3 direction;

const float PI = 3.14159265359f;
float shininess = 1.;

out vec4 color;


in VS_OUT {
    vec3 _normal;
    vec3 _color;
    vec2 _texCoor;
    vec3 _view;
    vec3 _position;
}fs_in;


struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

layout (binding = 0, std140) buffer lights{
    uint size;
    PointLight list[];
};



vec2 ray_box_intersect(vec3 ro, vec3 rd) {
  vec3 t0 = (bb_min - ro) / rd;
  vec3 t1 = (bb_max - ro) / rd;
  vec3 tmin = min(t0, t1);
  vec3 tmax = max(t0, t1);
 
  float dst_a = max(max(tmin.x, tmin.y), tmin.z);
  float dst_b = min(tmax.x, min(tmax.y, tmax.z));
 
  float dst_to_box = max(0., dst_a);
  float dst_through_box = max(0., dst_b - dst_to_box);
 
  return vec2(dst_to_box, dst_through_box);
}

float CalcScattering(float cosTheta, float G)
{
    return 3.f * (1.f - G * G) * (1.f + cosTheta * cosTheta) / (2.f *  (2 + G * G) * pow(1.f + G * G - 2.f * G * cosTheta, 1.5));
}

float CalcRayleighScattering(float cosTheta)
{
    return 3.f * (1.f + cosTheta * cosTheta) / (4.f);
}

vec4 CalcVolumeScattering(vec3 viewDir, PointLight light, float G, vec3 color, vec3 fragPos)
{
    vec3 origin = camera;
    float depth = 1000.;
 
    vec2 intersects = ray_box_intersect(origin, -viewDir);
    float dst_to = intersects.x;
    float dst_through = min(intersects.y, depth - dst_to);

    float stepSize = dst_through / float(NUM_STEPS_INT-1);
    vec3 step = -viewDir * stepSize;
    float density = 1.f;
    float inscattering = exp(0.f);
    float k = .05f;

    vec3 position = origin - viewDir * dst_to;
    vec3 volumetric = vec3(0.0f);
    float alpha = 0.f;
    for (int i = 0; i < NUM_STEPS_INT; ++i)
    {
        vec3 lightDir = normalize(light.position - position);
     
         float distance    = length(light.position - position);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));

     //volumetric += density * (CalcScattering(dot(lightDir, viewDir), G)) * color;// + CalcRayleighScattering(dot(lightDir, viewDir)) * vec3(0, 0, 1.));// * exp(k*-length(camera-position));     
     volumetric += density * ((CalcScattering(dot(lightDir, viewDir), G)) * color + CalcRayleighScattering(dot(lightDir, viewDir)) * vec3(0, 0, 1.))/2.f;// * exp(k*-length(camera-position));
     //volumetric *= attenuation;
        alpha += density * k * exp(-length(step));

        position += step;
    }
    //alpha /= float(NUM_STEPS_INT);
    volumetric /= float(NUM_STEPS_INT);
    volumetric *= inscattering;
    
    return vec4(volumetric, alpha);
}

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient  = light.ambient * 0.25;
    vec3 diffuse  = light.diffuse  * diff * 0.25;
    vec3 specular = light.specular * spec * 0.25;

    vec3 lightColor = vec3(1.f, 1.f, 1.f);
    vec4 volumetric = CalcVolumeScattering(viewDir, light, G, lightColor, fragPos);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    
    vec4 color = vec4(ambient + diffuse + specular + volumetric.xyz * 0.25, volumetric.w);
    //vec4 color = volumetric;

    return vec4(color.xyz, color.w);
}


void main()
{
    vec3 norm = normalize(fs_in._normal);
    vec3 viewDir = normalize(camera - fs_in._position);
    vec4 result = vec4(0., 0., 0., 0.);
   
   for(int i = 0; i < size; i++)
        result += CalcPointLight(list[i], norm, fs_in._position, viewDir);

   color = result;
}
