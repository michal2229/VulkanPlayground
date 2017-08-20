#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define SOFTEN_AO     25.0f
#define AMBIENT_COEFF 0.001f
#define PLANET_RADIUS 2.5f
#define LIGHT_RADIUS 0.4f

layout (binding = 1) uniform sampler2D samplerColorMap;

layout (location = 0) in vec3  inNormal;
layout (location = 1) in vec3  inColor;
layout (location = 2) in vec2  inUV;
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

float isFragShadedByObstacle(vec3 obstaclePos, vec3 fragPos, vec3 lightVec)
{
    vec3  lightPos = fragPos.xyz + lightVec;

    vec3  vecLiPl = obstaclePos - lightPos;  // vec from light to planet
    float lenLiPl = length(vecLiPl);

    vec3  vecLiRo = -lightVec;  // vec from light to rock
    float lenLiRo = length(vecLiRo);

    float k = LIGHT_RADIUS/(LIGHT_RADIUS + PLANET_RADIUS);
    vec3  lightPosNearF = lightPos + k*normalize(vec3(0.0f) - lightPos)*lenLiPl;
    vec3  lightPosFarF  = lightPos - k*normalize(vec3(0.0f) - lightPos)*lenLiPl;

    // Light pos near.
    vec3  vecLiPlNear = obstaclePos - lightPosNearF;  // vec from light to planet
    float lenLiPlNear = length(vecLiPlNear);

    vec3  vecLiRoNear = fragPos-lightPosNearF;  // vec from light to rock
    float lenLiRoNear = length(vecLiRoNear);

    float cosFiRoPlNear = dot(vecLiPlNear, vecLiRoNear) / (lenLiPlNear * lenLiRoNear);
    float fiRoPlNear = acos(cosFiRoPlNear);             // rock-planet angular distance from light point of view
    float fiPlRadNear = asin(PLANET_RADIUS/lenLiPlNear); // angular size of planet from light point of view


    // Light pos near.
    vec3  vecLiPlFar = obstaclePos - lightPosFarF;  // vec from light to planet
    float lenLiPlFar = length(vecLiPlFar);

    vec3  vecLiRoFar = fragPos-lightPosFarF;  // vec from light to rock
    float lenLiRoFar = length(vecLiRoFar);

    float cosFiRoPlFar = dot(vecLiPlFar, vecLiRoFar) / (lenLiPlFar * lenLiRoFar);
    float fiRoPlFar = acos(cosFiRoPlFar);             // rock-planet angular distance from light point of view
    float fiPlRadFar = asin(PLANET_RADIUS/lenLiPlFar); // angular size of planet from light point of view

    // Light pos center of light.
    float cosFiRoPl = dot(vecLiPl, vecLiRo) / (lenLiPl * lenLiRo);
    float fiRoPl = acos(cosFiRoPl);             // rock-planet angular distance from light point of view
    float fiPlRad = asin(PLANET_RADIUS/lenLiPl); // angular size of planet from light point of view

//    return ((fiRoPl < fiPlRad) && (lenLiRo > lenLiPl)) ? 1.0f : 0.0f;
    return min(max((fiPlRadNear - fiRoPl)/(fiPlRadNear - fiPlRadFar), 0.0f), 1.0f);
}

void main() 
{
	vec4 color = texture(samplerColorMap, inUV) * vec4(inColor, 1.0);
	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-L, N);
	
    vec3 ambient = inLightInt * AMBIENT_COEFF * vec3(1.0f) / (length(inLightVec) + SOFTEN_AO);
	vec3 diffuse = max(dot(N, L), 0.0) * inColor;
	vec3 specular = pow(max(dot(R, V), 0.0), 24.0) * vec3(1.0) * color.r;
    float shadow = fuzzNot( isFragShadedByObstacle(vec3(0.0f), intWorldPos, inLightVec) );

    outFragColor = vec4((diffuse * shadow + ambient) * color.rgb + specular * shadow, 1.0);
	outFragColor *= inLightInt;
    outFragColor /= length(inLightVec);
}
