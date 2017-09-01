#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include "VulkanTexture.hpp"
#include "VulkanModel.hpp"

// Element of scene definition.
struct Entity3dCreateInfo
{
    std::string entityName;
    std::string modelName;
    std::string textureName;
    std::string vertShaderName;
    std::string fragShaderName;
};

// Helper struct for grouping objects by drwable element.
struct Entity3dTraitsSet
{
    vks::Texture2D*  texturePtr;
    vks::Model*      modelPtr;
    VkPipeline*      vkPipelinePtr;
    VkDescriptorSet* vkDescriptorSetPtr;
};
