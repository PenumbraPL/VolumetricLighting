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

uniform bool isTexture = true;
uniform vec3 camera;

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

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    

    //vec3 ambient  = light.ambient  * vec3(texture(amb_tex, fs_in._texCoor));
    //vec3 diffuse  = light.diffuse  * diff * vec3(texture(dif_tex, fs_in._texCoor));
    //vec3 specular = light.specular * spec * vec3(texture(sp_tex, fs_in._texCoor));

    vec3 ambient  = light.ambient  * vec3(texture(alb_tex, fs_in._texCoor));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(sp_dif_tex, fs_in._texCoor));
    vec3 specular = light.specular * spec * vec3(texture(mr_tex, fs_in._texCoor));

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}



void main()
{
/*
    if(isTexture){
        color = texture(alb_tex, fs_in._texCoor);
    }else{
        color = vec4(fs_in._normal, 1.0);
    }

*/
vec3 norm = normalize(fs_in._normal);
vec3 viewDir = normalize(camera - fs_in._position);
vec3 result = vec3(0., 0., 0.);
for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, fs_in._position, viewDir);

color = vec4(result, 1.0);
//color = vec4(viewDir, 1.0);
}
