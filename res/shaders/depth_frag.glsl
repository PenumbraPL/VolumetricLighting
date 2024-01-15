#version 450 core

uniform vec3 camera;
uniform int NUM_STEPS_INT = 20;
uniform float SPECULAR_FACTOR = 16.0f;
uniform float G = -0.8f;
uniform vec3 bb_min = vec3(-1., -1., -1.);
uniform vec3 bb_max = vec3(1., 1., 1.);
//uniform vec3 direction;
float d = 2;//length(bb_max-bb_min);

const float PI = 3.14159265359f;

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



vec2 ray_box_intersect(vec3 origin, vec3 direction) {
  vec3 t0 = (bb_min - origin) / direction;
  vec3 t1 = (bb_max - origin) / direction;
  vec3 tmin = min(t0, t1);
  vec3 tmax = max(t0, t1);
 
  float dist_a = max(max(tmin.x, tmin.y), tmin.z);
  float dist_b = min(tmax.x, min(tmax.y, tmax.z));
 
  float dist_to = max(0., dist_a);
  float dist_through = max(0., dist_b - dist_to);
 
  return vec2(dist_to, dist_through);
}

float CalcMieScatter(float cosTheta, float G)
{
    return 3.f * (1.f - G * G) * (1.f + cosTheta * cosTheta) / (2.f *  (2 + G * G) * pow(1.f + G * G - 2.f * G * cosTheta, 1.5));
}

float CalcRayleighScatter(float cosTheta)
{
    return 3.f * (1.f + cosTheta * cosTheta) / (4.f);
}

vec4 CalcVolumeScatter(vec3 viewDir, PointLight light, float G, vec3 color, vec3 fragPos)
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
    float k = 1.f;

    vec3 position = origin - viewDir * dst_to;
    vec3 volumetric = vec3(0.0f);
    float alpha = 0.f;
    for (int i = 0; i < NUM_STEPS_INT; ++i)
    {
        vec3 lightDir = normalize(light.position - position);
     
         float distance    = length(light.position - position);
        float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));

        volumetric += density * ((CalcMieScatter(dot(lightDir, viewDir), G)) * color + CalcRayleighScatter(dot(lightDir, viewDir)) * vec3(1., 1., 1.))/2.f;
         //volumetric *= attenuation;

        position += step;
    }
    //alpha /= float(NUM_STEPS_INT);
    volumetric /= float(NUM_STEPS_INT);
    volumetric *= inscattering;
    
    alpha = density * k * (1.3-1.3*exp(-dst_through/d));

    return vec4(volumetric, alpha);
}

vec4 CalcLight(PointLight light, vec3 N, vec3 fragPos, vec3 V_dir)
{
    vec3 L_dir = normalize(light.position - fragPos);
    vec3 R_dir = reflect(-L_dir, N);

    float d = max(dot(N, L_dir), 0.0);
    const int shininess = 0;
    float s = pow(max(dot(V_dir, R_dir), 0.0), shininess);

    vec3 ambient  = 0.25 * light.ambient;
    vec3 diffuse  = 0.25 * light.diffuse  * d;
    vec3 specular = 0.25 * light.specular * s;

    vec3 lightColor = vec3(1.f, 1.f, 1.f);
    vec4 volumetric = CalcVolumeScatter(V_dir, light, G, lightColor, fragPos);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    
    vec4 color = vec4(diffuse + specular + ambient + volumetric.xyz * 0.25, volumetric.w);

    return vec4(color.xyz, color.w);
}


void main()
{
    vec3 norm = normalize(fs_in._normal);
    vec3 viewDir = normalize(camera - fs_in._position);
    vec4 result = vec4(0., 0., 0., 0.);
   
   for(int i = 0; i < size; i++)
        result += CalcLight(list[i], norm, fs_in._position, viewDir);

   color = result;
}
