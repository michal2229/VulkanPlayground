#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define SOFTEN_AO     25.0f
#define AMBIENT_COEFF 0.01f

#define PLANET_RADIUS 1.9f

layout (binding = 1) uniform sampler2DArray samplerArray;

layout (location = 0) in vec3  inNormal;
layout (location = 1) in vec3  inColor;
layout (location = 2) in vec3  inUV;
layout (location = 3) in vec3  inViewVec;
layout (location = 4) in vec3  inLightVec;
layout (location = 5) in float inLightInt;
layout (location = 6) in vec3  intWorldPos;

layout (location = 0) out vec4 outFragColor;

float fuzzAnd(float f0, float f1)
{
    return min(f0, f1);
}

float fuzzOr(float f0, float f1)
{
    return max(f0, f1);
}

float fuzzNot(float f)
{
    return 1.0f - f;
}

float isFragShadedByPlanet(vec3 planetPos, float planetRadius, vec3 rockPos, vec3 lightVec)
{
    vec3 lightPos = rockPos.xyz + lightVec;
    
    vec3 vecLiPl = planetPos - lightPos;  // vec from light to planet
    float lenLiPl = length(vecLiPl);
    
    vec3 vecLiRo = -lightVec;  // vec from light to rock
    float lenLiRo = length(vecLiRo);
    
    float cosFiRoPl = dot(vecLiPl, vecLiRo) / (lenLiPl * lenLiRo);
    float fiRoPl = acos(cosFiRoPl);             // cos of rock-planet angular distance from light point of view
    float fiPlRad = asin(planetRadius/lenLiPl); // angular size of planet from light point of view
    
    return ((fiRoPl < fiPlRad) && (lenLiRo > lenLiPl)) ? 1.0f : 0.0f;
}

void main() 
{
	vec4 color = texture(samplerArray, inUV) * vec4(inColor, 1.0);	
	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-L, N);
	
	vec3 ambient = inLightInt * AMBIENT_COEFF * vec3(1.0f) / (length(inLightVec) * length(inLightVec) + SOFTEN_AO);
	vec3 diffuse = max(dot(N, L), 0.0) * inColor;
	vec3 specular = (dot(N,L) > 0.0) ? pow(max(dot(R, V), 0.0), 16.0) * vec3(1.0) * color.r : vec3(0.0);
	float shadow = fuzzNot( isFragShadedByPlanet(vec3(0.0f), PLANET_RADIUS, intWorldPos, inLightVec) );
	
	outFragColor = vec4(ambient * color.rgb + diffuse * color.rgb * shadow + specular * shadow, 1.0);
	outFragColor *= inLightInt;
	outFragColor /= length(inLightVec) * length(inLightVec);
}
