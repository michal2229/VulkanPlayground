#pragma once

#include <assert.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <map>
#include <VulkanTexture.hpp>
#include <VulkanModel.hpp>
#include <HelperStructsAndFuncs.hpp>

namespace vk229
{

struct OffscreenViewportData {
//    vk229::entity_name_t       entityName     = "NOT_YET_FILLED";
//    vk229::mesh_name_t         meshName       = "NOT_YET_FILLED";
//    vk229::mesh_objtype_t*     pMeshObj       = nullptr;
//    vk229::textures_set_name_t baseTexSetName = "NOT_YET_FILLED";
//    struct TexturePtrSet {
//        vk229::texture_objtype_t* pTexCol    = nullptr; // Here be screen!!11
//        vk229::texture_objtype_t* pTexDiffDI = nullptr;
//        vk229::texture_objtype_t* pTexAO     = nullptr;
//        vk229::texture_objtype_t* pTexEmit   = nullptr;
//        vk229::texture_objtype_t* pTexNormal = nullptr;
//        vk229::texture_objtype_t* pTexRefl   = nullptr;
//    } texPtrSet;
//    vk229::shaders_set_name_t  shadersSetName = "NOT_YET_FILLED";
//    struct ShaderPtrSet {
//        VkPipelineShaderStageCreateInfo* pVertShadSCI = nullptr;
//        VkPipelineShaderStageCreateInfo* pFragShadSCI = nullptr;
//    } shaderPtrSet;

//    VkDescriptorSetLayout* pDescSetlayout = nullptr;
//    VkDescriptorPool  descriptorPool;
//    VkDescriptorSet   descriptorSet;

    // RENDERING OFFSCREEN {
    struct OffscreenRenderPass {

        struct OffscreenFramebufferAttachmentsDescription {
            std::pair<uint32_t, uint32_t> imagDim;
            struct OffscreenImageDescription {
                VkFormat                format;
                VkImageCreateInfo       imgCrInf;
                VkMemoryAllocateInfo    memAllInf;
                VkMemoryRequirements    memReq;
                VkImageViewCreateInfo   imViCrInf;
                VkAttachmentDescription attDesc;
                VkAttachmentReference   attRef;
            } col, dep;
        } oFbAttDesc;

        struct OffscreenFramebufferAttachments {
            struct OffscreenImage {
                VkDeviceMemory  mem;
                VkImage         image;
                VkImageView     imVi;
            } col, dep;
        } oFbAtt;

        struct OffscreenFramebuffer {
            VkFramebufferCreateInfo fbCrInf;
            VkFramebuffer           fb;
        } oFb;

        struct SubpassDescription {
            VkSubpassDescription    subDesc;
            VkSubpassDependency     inSubDep, outSubDep;
        } oSubDesc;

        struct RenderPass {
            VkRenderPassCreateInfo  rendPassCrInf;
            VkRenderPass            rendPass;
        } oRendPass;

        struct OffscreenSampler {
            VkSamplerCreateInfo smplrCrInf;
            VkSampler           smplr;
        } oSampler;

        VkDescriptorImageInfo colImgDescr; // { VkSampler, VkImageView, VkImageLayout}
        VkCommandBuffer oCmdBuff;
        VkSemaphore     oSmphre;
    } oRend;

    virtual void prepare(vks::VulkanDevice*, VkCommandPool&);
    virtual void recordCmdBuff(vks::VulkanDevice*, vk229::SceneData&);
};

} // namespace vk229
