#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColor;

layout (binding = 0) uniform UBO 
{
    mat4 view;
    mat4 projection;
    vec4 lightPos;
    vec4 camPos;
    float lightInt;
    float locSpeed;
    float globSpeed;
} ubo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;
layout (location = 5) out float outLightInt;


void main() 
{
	outColor = inColor;
	outUV = inUV * vec2(5.0, 3.0); // * vec2(10.0, 6.0) makes texture repetition
	outLightInt = ubo.lightInt;
	gl_Position = ubo.projection * ubo.view * vec4(inPos, 1.0);
	
	vec4 pos  = (vec4(inPos, 1.0));
	outNormal = (vec4(inNormal, 0.0)).xyz;
	vec4 cPos = (ubo.camPos); 
	vec4 lPos = (ubo.lightPos);
	outLightVec = (lPos - pos).xyz;
	outViewVec = (cPos - pos).xyz;		
}
