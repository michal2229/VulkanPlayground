#pragma once

#include <assert.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <map>
#include <VulkanTexture.hpp>
#include <VulkanModel.hpp>

namespace vk229
{

struct OffscreenViewportData {
    vk229::entity_name_t       entityName     = "NOT_YET_FILLED";
    vk229::mesh_name_t         meshName       = "NOT_YET_FILLED";
    vk229::mesh_objtype_t*     pMeshObj       = nullptr;
    vk229::textures_set_name_t baseTexSetName = "NOT_YET_FILLED";
    struct TexturePtrSet {
        vk229::texture_objtype_t* pTexCol    = nullptr; // Here be screen!!11
        vk229::texture_objtype_t* pTexDiffDI = nullptr;
        vk229::texture_objtype_t* pTexAO     = nullptr;
        vk229::texture_objtype_t* pTexEmit   = nullptr;
        vk229::texture_objtype_t* pTexNormal = nullptr;
        vk229::texture_objtype_t* pTexRefl   = nullptr;
    } texPtrSet;
    vk229::shaders_set_name_t  shadersSetName = "NOT_YET_FILLED";
    struct ShaderPtrSet {
        VkPipelineShaderStageCreateInfo* pVertShadSCI = nullptr;
        VkPipelineShaderStageCreateInfo* pFragShadSCI = nullptr;
    } shaderPtrSet;

    VkDescriptorSetLayout* pDescSetlayout = nullptr;
    VkDescriptorPool  descriptorPool;
    VkDescriptorSet   descriptorSet;

    VkRenderPass*     pRenderPass     = nullptr;
    VkFramebuffer*    pFramebuffer    = nullptr;
    VkPipelineLayout* pPipelineLayout = nullptr;
    VkPipeline*       pPipeline       = nullptr;

    VkCommandBuffer*  pCommandBuffer  = nullptr;
    VkCommandPool*    pCommandPool    = nullptr;

    VkQueue*          pQueue          = nullptr;

    VkDevice*         pDevice         = nullptr;
};

} // namespace vk229
