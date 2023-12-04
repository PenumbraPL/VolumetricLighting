#version 450 core


void main(void){
float pi = 3.14;
float D = (alpha*alpha*x + dot(N, H))/(pi*pow(dot(N, H)*dot(N, H)*(alpha*alpha-1)+1), 2);
float G = (2*len(dot(N, L))*x + dot(H, L))*(2*dot(N,V)*x + dot(H, V))/(len(dot(N, L))+sqrt(alpha*alpha+(1-alpha*alpha)*(dot(N, L)*dot(N,L)))*(dot(N, V)+sqrt(1-alpha*alpha)*dot(N, V)*dot(N, V));

vec3 black = (0., 0., 0.);
vec3 baseColor = texture(tex, tex_Coor);
float c_diff = lerp(baseColor, black, metallic);
float f0 = lerp(0.04, baseColor, metallic);
float alpha = roughness*roughness;

float F = f0 + (1 - f0) * pow(1 - abs(dot(V, H)), 5);
float f_diffuse = (1 - F) * (1/pi)*c_diff;
float f_specular = F * D * G / (4*abs(dot(V, N))* abs(dot(L, N)));

float material = f_diffuse + f_specular;
}