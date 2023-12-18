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

float near = 0.1;
float far = 500.;

in VS_OUT {
vec3 _normal;
vec3 _color;
vec2 _texCoor;
vec3 _view;
vec3 _position;
}fs_in;

float shininess = 1.;
uniform vec3 fog_color = vec3(0.7, 0.8, 0.9);


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


    vec3 ambient  = light.ambient  * vec3(texture(alb_tex, fs_in._texCoor));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(sp_dif_tex, fs_in._texCoor));
    vec3 specular = light.specular * spec * vec3(texture(mr_tex, fs_in._texCoor));

    float z = distance;
    float extinction = exp(-z);
	float inscattering = exp(-z);


    ambient  *= attenuation * extinction;
    diffuse  *= attenuation * extinction;
    specular *= attenuation * extinction;
    return (ambient + diffuse + specular + 3*(fog_color * (1. - inscattering)));
}


float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{

vec3 norm = normalize(fs_in._normal);
vec3 viewDir = normalize(camera - fs_in._position);
vec3 result = vec3(0., 0., 0.);
for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, fs_in._position, viewDir);

//color = fog(vec4(result, .5));
float depth = 0.;
if(!gl_FrontFacing) depth = LinearizeDepth(gl_FragCoord.z) / far;
color = vec4(depth, depth, depth, 1.);
}

vec4 fog(vec3 c){
	float z = length(fs_in._view);
	float de = 0.0025 * smoothstep(0.0, 6.0, 10.0 - fs_in._position.y);
	float di = 0.0045 * smoothstep(0.0, 40.0, 20.0 - fs_in._position.y);

	float extinction = exp(-z * de);
	float inscattering = exp(-z * di);

	return c * extinction + fog_color * (1.0 - inscattering);
}