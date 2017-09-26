#include <assert.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <map>
#include <VulkanTexture.hpp>
#include <VulkanModel.hpp>
#include <OffscreenHelpers.hpp>

void vk229::OffscreenViewportData::prepare(vks::VulkanDevice* _pDev, VkCommandPool& _cmdPool)
{
    auto& orp = this->oRend;

    { // Size.
        orp.oFbAttDesc.imagDim = {512u, 512u};
    }
    { // Depth Image.
        { // Format. Depth Image.
            VkFormat fbDepthFormat;
            VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(_pDev->physicalDevice, &fbDepthFormat);
            assert(validDepthFormat);
            orp.oFbAttDesc.dep.format = fbDepthFormat;
        }
        { // Image Create info. Depth Image.
            auto& [dim_x, dim_y] = orp.oFbAttDesc.imagDim;
            auto& format = orp.oFbAttDesc.dep.format;
            auto& image  = orp.oFbAttDesc.dep.imgCrInf;

            image = vks::initializers::imageCreateInfo();
            image.imageType     = VK_IMAGE_TYPE_2D;
            image.extent.width  = dim_x;
            image.extent.height = dim_y;
            image.extent.depth  = 1;
            image.mipLevels     = 1;
            image.arrayLayers   = 1;
            image.samples       = VK_SAMPLE_COUNT_1_BIT;
            image.tiling        = VK_IMAGE_TILING_OPTIMAL;
            image.format        = format;
            image.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        }
        { // Image. Depth Image.
            auto& imageCreateInfo = orp.oFbAttDesc.dep.imgCrInf;
            auto& image = orp.oFbAtt.dep.image;

            VK_CHECK_RESULT(vkCreateImage(_pDev->logicalDevice,
                                          &imageCreateInfo,
                                          nullptr,
                                          &image));
        }
        { // Memory allocation for image. Depth Image.
            auto& image         = orp.oFbAtt.dep.image;
            auto& imageMemory   = orp.oFbAtt.dep.mem;

            VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
            VkMemoryRequirements memReqs;

            vkGetImageMemoryRequirements(_pDev->logicalDevice, image, &memReqs);
            memAlloc.allocationSize = memReqs.size;
            memAlloc.memoryTypeIndex = _pDev->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_CHECK_RESULT(vkAllocateMemory(_pDev->logicalDevice, &memAlloc, nullptr, &imageMemory));
        }
        { // Binding memory to image. Depth Image.
            auto& image         = orp.oFbAtt.dep.image;
            auto& imageMemory   = orp.oFbAtt.dep.mem;

            VK_CHECK_RESULT(vkBindImageMemory(_pDev->logicalDevice, image, imageMemory, 0));
        }
        { // Image View create Info. Depth Image.
            auto& imageViewCreateInfo = orp.oFbAttDesc.dep.imViCrInf;
            auto& format = orp.oFbAttDesc.dep.format;
            auto& image  = orp.oFbAtt.dep.image;

            imageViewCreateInfo = vks::initializers::imageViewCreateInfo();
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = format;
            imageViewCreateInfo.flags = 0;
            imageViewCreateInfo.subresourceRange = {};
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            imageViewCreateInfo.image = image;
        }
        { // Image View. Depth Image.
            auto& imageViewCreateInfo = orp.oFbAttDesc.dep.imViCrInf;
            auto& imageView = orp.oFbAtt.dep.imVi;

            VK_CHECK_RESULT(vkCreateImageView(_pDev->logicalDevice, &imageViewCreateInfo, nullptr, &imageView));
        }
        { // Attachment description. Depth Image.
            auto& attachmentDescr = orp.oFbAttDesc.dep.attDesc;
            auto& format = orp.oFbAttDesc.dep.format;

            attachmentDescr.format          = format;
            attachmentDescr.samples         = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescr.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescr.storeOp         = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescr.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescr.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescr.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescr.finalLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        { // Arrachment reference. Depth Image.
            auto& attachmentRef = orp.oFbAttDesc.dep.attRef;
            attachmentRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
        }
    }
    { // Color Image.
        {
            VkFormat fbColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
            orp.oFbAttDesc.col.format = fbColorFormat;
        }
        { // Image Create info. Color Image.
            auto& [dim_x, dim_y] = orp.oFbAttDesc.imagDim;
            auto& format = orp.oFbAttDesc.col.format;
            auto& imageCreateInfo  = orp.oFbAttDesc.col.imgCrInf;

            imageCreateInfo = vks::initializers::imageCreateInfo();
            imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
            imageCreateInfo.extent.width  = dim_x;
            imageCreateInfo.extent.height = dim_y;
            imageCreateInfo.extent.depth  = 1;
            imageCreateInfo.mipLevels     = 1;
            imageCreateInfo.arrayLayers   = 1;
            imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
            imageCreateInfo.format        = format;
            imageCreateInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        { // Image. Color Image.
            auto& imageCreateInfo = orp.oFbAttDesc.col.imgCrInf;
            auto& image = orp.oFbAtt.col.image;

            VK_CHECK_RESULT(vkCreateImage(_pDev->logicalDevice,
                                          &imageCreateInfo,
                                          nullptr,
                                          &image));
        }
        { // Memory allocation for image. Color Image.
            auto& image         = orp.oFbAtt.col.image;
            auto& imageMemory   = orp.oFbAtt.col.mem;

            VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
            VkMemoryRequirements memReqs;

            vkGetImageMemoryRequirements(_pDev->logicalDevice, image, &memReqs);
            memAlloc.allocationSize = memReqs.size;
            memAlloc.memoryTypeIndex = _pDev->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_CHECK_RESULT(vkAllocateMemory(_pDev->logicalDevice, &memAlloc, nullptr, &imageMemory));
        }
        { // Binding memory to image. Color Image.
            auto& image         = orp.oFbAtt.col.image;
            auto& imageMemory   = orp.oFbAtt.col.mem;

            VK_CHECK_RESULT(vkBindImageMemory(_pDev->logicalDevice, image, imageMemory, 0));
        }
        { // Image View create Info. Color Image.
            auto& imageViewCreateInfo = orp.oFbAttDesc.col.imViCrInf;
            auto& format = orp.oFbAttDesc.col.format;
            auto& image  = orp.oFbAtt.col.image;

            imageViewCreateInfo = vks::initializers::imageViewCreateInfo();
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = format;
            imageViewCreateInfo.subresourceRange = {};
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            imageViewCreateInfo.image = image;
        }
        { // Image View. Color Image.
            auto& imageViewCreateInfo = orp.oFbAttDesc.col.imViCrInf;
            auto& imageView = orp.oFbAtt.col.imVi;

            VK_CHECK_RESULT(vkCreateImageView(_pDev->logicalDevice, &imageViewCreateInfo, nullptr, &imageView));
        }
        { // Attachment description. Color Image.
            auto& attachmentDescr = orp.oFbAttDesc.col.attDesc;
            auto& format = orp.oFbAttDesc.col.format;

            attachmentDescr.format          = format;
            attachmentDescr.samples         = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescr.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescr.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescr.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescr.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescr.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescr.finalLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        { // Attachment reference. Color Image.
            auto& attachmentRef = orp.oFbAttDesc.col.attRef;
            attachmentRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        }
    }
    { // Subpass.
        auto& subpassDescription = orp.oSubDesc.subDesc;
        auto& subpassInDep  = orp.oSubDesc.inSubDep;
        auto& subpassOutDep = orp.oSubDesc.outSubDep;
        auto& depthAttachmentRef = orp.oFbAttDesc.dep.attRef;
        auto& colorAttachmentRef = orp.oFbAttDesc.col.attRef;

        subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments        = &colorAttachmentRef;
        subpassDescription.pDepthStencilAttachment  = &depthAttachmentRef;

        subpassInDep.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassInDep.dstSubpass = 0;
        subpassInDep.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassInDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassInDep.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassInDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassInDep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        subpassOutDep.srcSubpass = 0;
        subpassOutDep.dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassOutDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassOutDep.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassOutDep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassOutDep.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassOutDep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    }
    {// Render Pass.
        auto& renderPassCreateInfo = orp.oRendPass.rendPassCrInf;
        auto& renderPass = orp.oRendPass.rendPass;
        auto& colorAttDescr = orp.oFbAttDesc.col.attDesc;
        auto& depthAttDescr = orp.oFbAttDesc.dep.attDesc;
        auto& subpassDescription = orp.oSubDesc.subDesc;
        auto& subpassInDep  = orp.oSubDesc.inSubDep;
        auto& subpassOutDep = orp.oSubDesc.outSubDep;

        std::array<VkAttachmentDescription, 2> attachmentDescriptions = {colorAttDescr, depthAttDescr};
        std::array<VkSubpassDependency, 2> dependencies = {subpassInDep, subpassOutDep};

        renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
        renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        renderPassCreateInfo.dependencyCount = dependencies.size();
        renderPassCreateInfo.pDependencies   = dependencies.data();

        VK_CHECK_RESULT(vkCreateRenderPass(_pDev->logicalDevice, &renderPassCreateInfo, nullptr, &renderPass));
    }
    { // Framebuffer.
        auto& [dim_x, dim_y] = orp.oFbAttDesc.imagDim;
        auto& renderPass = orp.oRendPass.rendPass;
        auto& framebufferCreateInfo = orp.oFb.fbCrInf;
        auto& framebuffer = orp.oFb.fb;
        auto& colorAtt = orp.oFbAtt.col.imVi;
        auto& depthAtt = orp.oFbAtt.dep.imVi;

        std::array<VkImageView, 2> attachments = {colorAtt, depthAtt};

        framebufferCreateInfo = vks::initializers::framebufferCreateInfo();
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.attachmentCount = attachments.size();
        framebufferCreateInfo.pAttachments = attachments.data();
        framebufferCreateInfo.width  = dim_x;
        framebufferCreateInfo.height = dim_y;
        framebufferCreateInfo.layers = 1;

        VK_CHECK_RESULT(vkCreateFramebuffer(_pDev->logicalDevice, &framebufferCreateInfo, nullptr, &framebuffer));
    }
    { // Sampler.
        auto& samplerCreateInfo = orp.oSampler.smplrCrInf;
        auto& sampler = orp.oSampler.smplr;

        samplerCreateInfo= vks::initializers::samplerCreateInfo();
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
        samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        VK_CHECK_RESULT(vkCreateSampler(_pDev->logicalDevice, &samplerCreateInfo, nullptr, &sampler));
    }
    { // Descriptors.
        auto& descriptors = orp.colImgDescr;
        auto& view        = orp.oFbAtt.col.imVi;
        auto& sampler     = orp.oSampler.smplr;

        descriptors.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        descriptors.imageView   = view;
        descriptors.sampler     = sampler;
    }
    { // Command buffer.
        auto& cmdBuffer = orp.oCmdBuff;

        VkCommandBufferAllocateInfo cmdBufAllocateInfo =
            vks::initializers::commandBufferAllocateInfo(
                _cmdPool,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                1);

        VK_CHECK_RESULT(vkAllocateCommandBuffers(_pDev->logicalDevice, &cmdBufAllocateInfo, &cmdBuffer));
    }
}

void vk229::OffscreenViewportData::recordCmdBuff(vks::VulkanDevice* _pDev, vk229::SceneData& _sd)
{
    auto& orp = this->oRend;

    auto& [dim_x, dim_y] = orp.oFbAttDesc.imagDim;
    auto& commandbuffer = orp.oCmdBuff;
    auto& semaphore     = orp.oSmphre;
    auto& renderPass = orp.oRendPass.rendPass;
    auto& framebuffer = orp.oFb.fb;

    if (semaphore == VK_NULL_HANDLE)
    {
        // Create a semaphore used to synchronize offscreen rendering and usage
        VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
        VK_CHECK_RESULT(vkCreateSemaphore(_pDev->logicalDevice, &semaphoreCreateInfo, nullptr, &semaphore));
    }

    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    VkClearValue clearValues[2];
    clearValues[0].color = { { 0.5f, 0.5f, 0.5f, 0.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = framebuffer;
    renderPassBeginInfo.renderArea.extent.width  = dim_x;
    renderPassBeginInfo.renderArea.extent.height = dim_y;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    VK_CHECK_RESULT(vkBeginCommandBuffer(commandbuffer, &cmdBufInfo));

    vkCmdBeginRenderPass(commandbuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = vks::initializers::viewport((float)dim_x, (float)dim_y, 0.0f, 1.0f);
    vkCmdSetViewport(commandbuffer, 0, 1, &viewport);

    VkRect2D scissor = vks::initializers::rect2D(dim_x, dim_y, 0, 0);
    vkCmdSetScissor(commandbuffer, 0, 1, &scissor);

    VkDeviceSize offsets[1] = { 0 };

    { // RECORDING COMMMAND BUFFER
        _sd.recordDrawCommandsForEntities(commandbuffer, 0, offsets);
    }

    vkCmdEndRenderPass(commandbuffer);

    VK_CHECK_RESULT(vkEndCommandBuffer(commandbuffer));
}
