#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// Layout of these vertex attributes is defined in preparePipelines().
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTan;
layout (location = 3) in vec3 inBiTan;
layout (location = 4) in vec2 inUV;
layout (location = 5) in vec3 inColor;

// Layout of these bindings is defined in setupDescriptorSetLayout().
layout (binding = 0) uniform UBO 
{
    mat4 view;
    mat4 projection;
//    vec4 camPos;
} ubo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outTan;
layout (location = 2) out vec3 outBiTan;
layout (location = 3) out vec2 outUV;
layout (location = 4) out vec3 outColor;
layout (location = 5) out vec3 outViewVec;


void main() 
{
    vec4 camPos = inverse(ubo.view) * vec4(0.0f, 0.0f, 0.0f, 1.0f);

    gl_Position = ubo.projection * ubo.view * vec4(inPos, 1.0);
    outNormal   = inNormal;
    outColor    = inColor;
    outUV       = inUV * vec2(1.0, -1.0);
    outViewVec  = camPos.xyz - inPos;
    outTan      = inTan;
    outBiTan    = inBiTan;

}
