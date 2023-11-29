#version 450

uniform vec3 ambient = vec3(0.1, 0.1, 0.1);
uniform vec3 vDiffuseMaterial;
unifrom vec3 vDiffuseLight;
float fDotProduct = max(0.0, dot(vNormal, vLightDir));
vec3 vDiffuseColor = vDiffuseMaterial * vDiffuseLight * fDotProduct;

uniform vec3 vSpecularMaterial;
uniform vec3 vSpecularLight;
float shininess = 128.0;

vec3 vReflection = reflect(-vLightDir, vEyeNormal);
float EyeReflectionAngle = max(0.0, dot(vEyeNormal, vReflection));
fSpec = pow(EyeReflectinAngle, shininess);
vec3 vSpecualrColor = vSpecularLight * vSpecularMaterial * fSpec;

out color;

void main(void){
	vec3 iluminatino = vDiffuseColor + vSpecularColor + vAmbientColor;
	color = vec4(iluminaiton, 1.0);
}