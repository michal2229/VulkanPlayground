#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define SOFTEN_AO     25.0f
#define AMBIENT_COEFF 0.01f

layout (binding = 1) uniform sampler2D samplerColorMap;

layout (location = 0) in vec3  inNormal;
layout (location = 1) in vec3  inColor;
layout (location = 2) in vec2  inUV;
layout (location = 3) in vec3  inViewVec;
layout (location = 4) in vec3  inLightVec;
layout (location = 5) in float inLightInt;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 color = texture(samplerColorMap, inUV) * vec4(inColor, 1.0) * (inLightInt);

	outFragColor = color;
}
