#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <map>
#include "VulkanTexture.hpp"
#include "VulkanModel.hpp"

// Element of scene definition.
struct Entity3dCreateInfo
{
    std::string entityName;
    std::string modelName;
    std::map<std::string, std::string> textureMap;
    std::map<std::string, std::string> shadersMap;
};

// Helper struct for grouping objects by drawable element.
struct Entity3dTraitsSet
{
    std::map<std::string, vks::Texture2D*> texturePtrMap;
    vks::Model*      modelPtr;
    VkPipeline*      vkPipelinePtr;
    VkDescriptorSet* vkDescriptorSetPtr;
};
