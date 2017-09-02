#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D samplerColor;
layout (binding = 2) uniform sampler2D samplerDiffuseDI;
layout (binding = 3) uniform sampler2D samplerAO;
layout (binding = 4) uniform sampler2D samplerEmit;
layout (binding = 5) uniform sampler2D samplerNormal;
layout (binding = 6) uniform sampler2D samplerReflection;

layout (location = 0) in vec3  inNormal;
layout (location = 1) in vec3  inColor;
layout (location = 2) in vec2  inUV;
layout (location = 3) in vec3  inViewVec;

layout (location = 0) out vec4 outFragColor;

#define AO_COEFF 0.5f

void main() 
{
    // { Computing vectors.
    vec3 N = normalize(inNormal);
    vec3 V = normalize(inViewVec);
    vec3 R = reflect(-V, N);
    // }

    // { Computing UV coords for reflection texture.
    float reflTh = acos(R.y);     // Theta //     0 .. pi
    float reflFi = atan(R.z/R.x); // Phi   // -pi/2 .. pi/2
    float angleOfRefl = acos(dot(vec2(R.x/R.z), vec2(-V.x, -V.z)));
    vec2  reflUV = vec2(0.5f-reflFi/6.28f, 1.0f-reflTh/3.14f);
    // }

    // { Computing fresnel coefficient.
    float met = 0.2f; // metalness
    float dot = max( 0.0f, dot( N, V ) );
    float fresnel = min(1.0f, met + ( 1.0f - met ) * pow( ( 1.0f - dot ), 5.0f ));
    // }

    // { Computing textures colors.
    vec4 COL  = texture(samplerColor,     inUV);
    vec4 DDI  = texture(samplerDiffuseDI, inUV);
    vec4 AO   = texture(samplerAO,        inUV);
    vec4 EMIT = texture(samplerEmit,      inUV);
    vec4 NORM = texture(samplerNormal,    inUV);
    vec4 REFLECT = texture(samplerReflection, reflUV);
    // }

    // Compositing final fragment color.
    outFragColor = (1.0f - met) * COL * (DDI + AO*AO_COEFF) + EMIT + REFLECT * fresnel;
}
