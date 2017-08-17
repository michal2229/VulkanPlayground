/*
* Vulkan Example - Instanced mesh rendering, uses a separate vertex buffer for instanced data
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h> 
#include <vector>
#include <random>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include "vulkanexamplebase.h"
#include "VulkanBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanModel.hpp"

#define VERTEX_BUFFER_BIND_ID   0
#define INSTANCE_BUFFER_BIND_ID 1
#define DESCRIPTOR_COUNT        3
#define ENABLE_VALIDATION       false
#define LIGHT_INTENSITY         70
#define INSTANCE_COUNT          2048
#define PLANET_SCALE            0.1f
#define LIGHT_SCALE             0.025f
#define INSTANCE_SCALE          0.2f

class VulkanExample : public VulkanExampleBase
{
public:
    struct {
        vks::Texture2DArray rocksTex2DArr;
        vks::Texture2D planetTex2D;
        vks::Texture2D lightTex2D;
    } textures;

    // Vertex layout for the models
    vks::VertexLayout vertexLayout = vks::VertexLayout({
        vks::VERTEX_COMPONENT_POSITION,
        vks::VERTEX_COMPONENT_NORMAL,
        vks::VERTEX_COMPONENT_UV,
        vks::VERTEX_COMPONENT_COLOR,
    });

    struct {
        vks::Model rockModel;
        vks::Model planetModel;
        vks::Model lightModel;
    } models;

    // Per-instance data block
    struct InstanceData {
        glm::vec3 pos;
        glm::vec3 rot;
        float scale;
        uint32_t texIndex;
    };
    // Contains the instanced data
    struct InstanceBuffer {
        VkBuffer buffer       = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        size_t size           = 0;
        VkDescriptorBufferInfo descriptor;
    } instanceBuffer;

    // M V P
    // M - MODEL MAT      - model space -> world space
    // V - VIEW MAT       - world space -> camera space
    // P - PROJECTION MAT - camera space -> square frustum space
    // MVP = P * V * M
    // gl_Position =  MVP * vec4(inPos, 1.0f);
    // someVector  =  MVP * vec4(inVec, 0.0f);
    struct UBOVS {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 lightPos = glm::vec4(0.707f*28.0f, -3.0f, -0.707f*28.0f, 1.0f);
        glm::vec4 camPos   = glm::vec4();
        float lightInt  = 0.0f;
        float locSpeed  = 0.0f;
        float globSpeed = 0.0f;
    } uboVS;

    struct {
        vks::Buffer scene;
    } uniformBuffers;

    VkPipelineLayout pipelineLayout;
    struct {
        VkPipeline instancedRocksVkPipeline;
        VkPipeline planetVkPipeline;
        VkPipeline lightVkPipeline;
        VkPipeline starfieldVkPipeline;
    } pipelines;

    VkDescriptorSetLayout descriptorSetLayout;
    struct {
        VkDescriptorSet instancedRocksVkDescrSet;
        VkDescriptorSet planetVkDescrSet;
        VkDescriptorSet lightVkDescrSet;
    } descriptorSets;

    VulkanExample() : VulkanExampleBase(ENABLE_VALIDATION)
    {
        title = "Vulkan Example - Instanced mesh rendering - 229";
        enableTextOverlay = true;
        srand(time(NULL));
        cameraPos = { 15.2f, -8.5f, 0.0f };
        rotation = {-520.0f, -2925.0f, 0.0f };
        zoom = -48.0f;
        rotationSpeed = 0.25f;
    }

    ~VulkanExample()
    {
        vkDestroyPipeline(device, pipelines.instancedRocksVkPipeline, nullptr);
        vkDestroyPipeline(device, pipelines.planetVkPipeline, nullptr);
        vkDestroyPipeline(device, pipelines.lightVkPipeline, nullptr);
        vkDestroyPipeline(device, pipelines.starfieldVkPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        vkDestroyBuffer(device, instanceBuffer.buffer, nullptr);
        vkFreeMemory(device, instanceBuffer.memory, nullptr);
        models.rockModel.destroy();
        models.planetModel.destroy();
        models.lightModel.destroy();
        textures.rocksTex2DArr.destroy();
        textures.planetTex2D.destroy();
        textures.lightTex2D.destroy();
        uniformBuffers.scene.destroy();
    }

    void buildCommandBuffers()
    {
        VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

        VkClearValue clearValues[2];
        clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.renderArea.extent.width = width;
        renderPassBeginInfo.renderArea.extent.height = height;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;

        for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
        {
            // Set target frame buffer
            renderPassBeginInfo.framebuffer = frameBuffers[i];

            VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
            vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

            VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
            vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

            VkDeviceSize offsets[1] = { 0 };

            // Star field
            vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.planetVkDescrSet, 0, NULL);
            vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.starfieldVkPipeline);
            vkCmdDraw(drawCmdBuffers[i], 4, 1, 0, 0);

            // Planet
            vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.planetVkDescrSet, 0, NULL);
            vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.planetVkPipeline);
            vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &models.planetModel.vertices.buffer, offsets);
            vkCmdBindIndexBuffer(drawCmdBuffers[i], models.planetModel.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(drawCmdBuffers[i], models.planetModel.indexCount, 1, 0, 0, 0);

            // Light
            vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.lightVkDescrSet, 0, NULL);
            vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.lightVkPipeline);
            vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &models.lightModel.vertices.buffer, offsets);
            vkCmdBindIndexBuffer(drawCmdBuffers[i], models.lightModel.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(drawCmdBuffers[i], models.lightModel.indexCount, 1, 0, 0, 0);

            // Instanced rocks
            vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.instancedRocksVkDescrSet, 0, NULL);
            vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.instancedRocksVkPipeline);
            // Binding point 0 : Mesh vertex buffer
            vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &models.rockModel.vertices.buffer, offsets);
            // Binding point 1 : Instance data buffer
            vkCmdBindVertexBuffers(drawCmdBuffers[i], INSTANCE_BUFFER_BIND_ID, 1, &instanceBuffer.buffer, offsets);

            vkCmdBindIndexBuffer(drawCmdBuffers[i], models.rockModel.indices.buffer, 0, VK_INDEX_TYPE_UINT32);

            // Render instances
            vkCmdDrawIndexed(drawCmdBuffers[i], models.rockModel.indexCount, INSTANCE_COUNT, 0, 0, 0);

            vkCmdEndRenderPass(drawCmdBuffers[i]);

            VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
        }
    }

    void loadAssets()
    {
        models.rockModel.loadFromFile(getAssetPath()       + "models/rock01.dae", vertexLayout, INSTANCE_SCALE, vulkanDevice, queue);
//        models.rock.loadFromFile(getAssetPath() + "models/4s.dae", vertexLayout, 0.025f, vulkanDevice, queue);
        models.planetModel.loadFromFile(getAssetPath()     + "models/sphere.obj", vertexLayout, PLANET_SCALE,   vulkanDevice, queue);
        models.lightModel.loadFromFile(getAssetPath() + "models/sphere.obj", vertexLayout, LIGHT_SCALE,    vulkanDevice, queue);

        // Textures
        std::string texFormatSuffix;
        VkFormat texFormat;
        // Get supported compressed texture format
        if (vulkanDevice->features.textureCompressionBC) {
            texFormatSuffix = "_bc3_unorm";
            texFormat = VK_FORMAT_BC3_UNORM_BLOCK;
        }
        else if (vulkanDevice->features.textureCompressionASTC_LDR) {
            texFormatSuffix = "_astc_8x8_unorm";
            texFormat = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        }
        else if (vulkanDevice->features.textureCompressionETC2) {
            texFormatSuffix = "_etc2_unorm";
            texFormat = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        }
        else {
            vks::tools::exitFatal("Device does not support any compressed texture format!", "Error");
        }

        textures.rocksTex2DArr.loadFromFile(getAssetPath() + "textures/texturearray_rocks" + texFormatSuffix + ".ktx", texFormat, vulkanDevice, queue);
        textures.planetTex2D.loadFromFile(getAssetPath()   + "textures/lava_from_gimp_planet_bc3_unorm.dds", VK_FORMAT_BC3_UNORM_BLOCK, vulkanDevice, queue);
        textures.lightTex2D.loadFromFile(getAssetPath()    + "textures/lava_from_gimp_light_bc3_unorm.dds", VK_FORMAT_BC3_UNORM_BLOCK, vulkanDevice, queue);
    }

    void setupDescriptorPool()
    {
        // Example uses one ubo
        std::vector<VkDescriptorPoolSize> poolSizes =
        {
            vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DESCRIPTOR_COUNT),
            vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESCRIPTOR_COUNT),
        };

        VkDescriptorPoolCreateInfo descriptorPoolInfo =
            vks::initializers::descriptorPoolCreateInfo(
                poolSizes.size(),
                poolSizes.data(),
                DESCRIPTOR_COUNT);

        VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
    }

    void setupDescriptorSetLayout()
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
        {
            // Binding 0 : Vertex shader uniform buffer
            vks::initializers::descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_SHADER_STAGE_VERTEX_BIT,
                0),
            // Binding 1 : Fragment shader combined sampler
            vks::initializers::descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                1),
        };

        VkDescriptorSetLayoutCreateInfo descriptorLayout =
            vks::initializers::descriptorSetLayoutCreateInfo(
                setLayoutBindings.data(),
                setLayoutBindings.size());

        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo(
                &descriptorSetLayout,
                1);

        VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
    }

    void setupDescriptorSet()
    {
        VkDescriptorSetAllocateInfo descripotrSetAllocInfo;
        std::vector<VkWriteDescriptorSet> writeDescriptorSets;

        descripotrSetAllocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);;

        // Instanced rocks
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descripotrSetAllocInfo, &descriptorSets.instancedRocksVkDescrSet));
        writeDescriptorSets = {
            vks::initializers::writeDescriptorSet(descriptorSets.instancedRocksVkDescrSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &uniformBuffers.scene.descriptor),	// Binding 0 : Vertex shader uniform buffer
            vks::initializers::writeDescriptorSet(descriptorSets.instancedRocksVkDescrSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textures.rocksTex2DArr.descriptor)	// Binding 1 : Color map
        };
        vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);

        // Planet
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descripotrSetAllocInfo, &descriptorSets.planetVkDescrSet));
        writeDescriptorSets = {
            vks::initializers::writeDescriptorSet(descriptorSets.planetVkDescrSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &uniformBuffers.scene.descriptor),			// Binding 0 : Vertex shader uniform buffer
            vks::initializers::writeDescriptorSet(descriptorSets.planetVkDescrSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textures.planetTex2D.descriptor)			// Binding 1 : Color map
        };
        vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);

        // Light
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descripotrSetAllocInfo, &descriptorSets.lightVkDescrSet));
        writeDescriptorSets = {
            vks::initializers::writeDescriptorSet(descriptorSets.lightVkDescrSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &uniformBuffers.scene.descriptor),			// Binding 0 : Vertex shader uniform buffer
            vks::initializers::writeDescriptorSet(descriptorSets.lightVkDescrSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textures.lightTex2D.descriptor)			// Binding 1 : Color map
        };
        vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);

    }

    void preparePipelines()
    {
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
            vks::initializers::pipelineInputAssemblyStateCreateInfo(
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                0,
                VK_FALSE);

        VkPipelineRasterizationStateCreateInfo rasterizationState =
            vks::initializers::pipelineRasterizationStateCreateInfo(
                VK_POLYGON_MODE_FILL,
                VK_CULL_MODE_BACK_BIT,
                VK_FRONT_FACE_CLOCKWISE,
                0);

        VkPipelineColorBlendAttachmentState blendAttachmentState =
            vks::initializers::pipelineColorBlendAttachmentState(
                0xf,
                VK_FALSE);

        VkPipelineColorBlendStateCreateInfo colorBlendState =
            vks::initializers::pipelineColorBlendStateCreateInfo(
                1,
                &blendAttachmentState);

        VkPipelineDepthStencilStateCreateInfo depthStencilState =
            vks::initializers::pipelineDepthStencilStateCreateInfo(
                VK_TRUE,
                VK_TRUE,
                VK_COMPARE_OP_LESS_OR_EQUAL);

        VkPipelineViewportStateCreateInfo viewportState =
            vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

        VkPipelineMultisampleStateCreateInfo multisampleState =
            vks::initializers::pipelineMultisampleStateCreateInfo(
                VK_SAMPLE_COUNT_1_BIT,
                0);

        std::vector<VkDynamicState> dynamicStateEnables = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState =
            vks::initializers::pipelineDynamicStateCreateInfo(
                dynamicStateEnables.data(),
                dynamicStateEnables.size(),
                0);

        // Load shaders
        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

        VkGraphicsPipelineCreateInfo pipelineCreateInfo =
            vks::initializers::pipelineCreateInfo(
                pipelineLayout,
                renderPass,
                0);

        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineCreateInfo.pRasterizationState = &rasterizationState;
        pipelineCreateInfo.pColorBlendState = &colorBlendState;
        pipelineCreateInfo.pMultisampleState = &multisampleState;
        pipelineCreateInfo.pViewportState = &viewportState;
        pipelineCreateInfo.pDepthStencilState = &depthStencilState;
        pipelineCreateInfo.pDynamicState = &dynamicState;
        pipelineCreateInfo.stageCount = shaderStages.size();
        pipelineCreateInfo.pStages = shaderStages.data();

        // This example uses two different input states, one for the instanced part and one for non-instanced rendering
        VkPipelineVertexInputStateCreateInfo inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        // Vertex input bindings
        // The instancing pipeline uses a vertex input state with two bindings
        bindingDescriptions = {
            // Binding point 0: Mesh vertex layout description at per-vertex rate
            vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, vertexLayout.stride(), VK_VERTEX_INPUT_RATE_VERTEX),
            // Binding point 1: Instanced data at per-instance rate
            vks::initializers::vertexInputBindingDescription(INSTANCE_BUFFER_BIND_ID, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE)
        };

        // Vertex attribute bindings
        // Note that the shader declaration for per-vertex and per-instance attributes is the same, the different input rates are only stored in the bindings:
        // instanced.vert:
        //	layout (location = 0) in vec3 inPos;			Per-Vertex
        //	...
        //	layout (location = 4) in vec3 instancePos;	Per-Instance
        attributeDescriptions = {
            // Per-vertex attributees
            // These are advanced for each vertex fetched by the vertex shader
            vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),					// Location 0: Position
            vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),	// Location 1: Normal
            vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6),		// Location 2: Texture coordinates
            vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 8),	// Location 3: Color
            // Per-Instance attributes
            // These are fetched for each instance rendered
            vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 5, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),	// Location 4: Position
            vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 4, VK_FORMAT_R32G32B32_SFLOAT, 0),					// Location 5: Rotation
            vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 6, VK_FORMAT_R32_SFLOAT,sizeof(float) * 6),			// Location 6: Scale
            vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 7, VK_FORMAT_R32_SINT, sizeof(float) * 7),			// Location 7: Texture array layer index
        };
        inputState.pVertexBindingDescriptions = bindingDescriptions.data();
        inputState.pVertexAttributeDescriptions = attributeDescriptions.data();

        pipelineCreateInfo.pVertexInputState = &inputState;

        // Instancing pipeline
        shaderStages[0] = loadShader(getAssetPath() + "shaders/instancing-229/instancing.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        shaderStages[1] = loadShader(getAssetPath() + "shaders/instancing-229/instancing.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
        // Use all input bindings and attribute descriptions
        inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.instancedRocksVkPipeline));

        // Planet rendering pipeline
        shaderStages[0] = loadShader(getAssetPath() + "shaders/instancing-229/planet.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        shaderStages[1] = loadShader(getAssetPath() + "shaders/instancing-229/planet.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
        // Only use the non-instanced input bindings and attribute descriptions
        inputState.vertexBindingDescriptionCount = 1;
        inputState.vertexAttributeDescriptionCount = 4;
        VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.planetVkPipeline));

        // Light rendering pipeline
        shaderStages[0] = loadShader(getAssetPath() + "shaders/instancing-229/light.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        shaderStages[1] = loadShader(getAssetPath() + "shaders/instancing-229/light.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
        // Only use the non-instanced input bindings and attribute descriptions
        inputState.vertexBindingDescriptionCount = 1;
        inputState.vertexAttributeDescriptionCount = 4;
        VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.lightVkPipeline));

        // Star field pipeline
        rasterizationState.cullMode = VK_CULL_MODE_NONE;
        depthStencilState.depthWriteEnable = VK_FALSE;
        shaderStages[0] = loadShader(getAssetPath() + "shaders/instancing-229/starfield.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        shaderStages[1] = loadShader(getAssetPath() + "shaders/instancing-229/starfield.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
        // Vertices are generated in the vertex shader
        inputState.vertexBindingDescriptionCount = 0;
        inputState.vertexAttributeDescriptionCount = 0;
        VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.starfieldVkPipeline));
    }

    float rnd(float range)
    {
        return range * (rand() / double(RAND_MAX));
    }

    void prepareInstanceData()
    {
        std::vector<InstanceData> instanceData;
        instanceData.resize(INSTANCE_COUNT);

        std::mt19937 rndGenerator(time(NULL));
        std::uniform_real_distribution<float> uniformDist(0.0, 1.0);

        // Distribute rocks randomly on two different rings

        std::vector<glm::vec2> rings = {
            {   5.0f,   7.0f },
            {   8.0f,  11.0f },
            {  13.0f,  17.0f },
            {  20.0f,  26.0f },
            {  30.0f,  40.0f },
            {  48.0f,  60.0f },
            {  75.0f, 100.0f },
            { 150.0f, 200.0f },
        };
        const auto numOfChunks = rings.size();
        const auto numInChunk  = INSTANCE_COUNT / rings.size();
        float rho, theta;

        for (auto instIdInChunk = 0; instIdInChunk < numInChunk; instIdInChunk++)
        {
            for (auto ringId = 0; ringId < numOfChunks; ringId++)
            {
                const auto instanceId    = instIdInChunk + ringId*numInChunk;
                auto& currentInstanceRef = instanceData[instanceId];

                rho   = sqrt((pow(rings.at(ringId)[1], 2.0f) - pow(rings.at(ringId)[0], 2.0f)) * uniformDist(rndGenerator) + pow(rings.at(ringId)[0], 2.0f));
                theta = 2.0 * M_PI * uniformDist(rndGenerator);

                currentInstanceRef.pos      = glm::vec3(rho*cos(theta), uniformDist(rndGenerator) * 0.05f - 0.25f, rho*sin(theta));
                currentInstanceRef.rot      = glm::vec3(M_PI * uniformDist(rndGenerator), M_PI * uniformDist(rndGenerator), M_PI * uniformDist(rndGenerator));
                currentInstanceRef.scale    = 1.5f + uniformDist(rndGenerator) - uniformDist(rndGenerator);
                currentInstanceRef.texIndex = rnd(textures.rocksTex2DArr.layerCount);
                currentInstanceRef.scale    *= 0.75f;
            }
        }

        instanceBuffer.size = instanceData.size() * sizeof(InstanceData);

        // Staging
        // Instanced data is static, copy to device local memory
        // This results in better performance

        struct {
            VkDeviceMemory memory;
            VkBuffer buffer;
        } stagingBuffer;

        VK_CHECK_RESULT(vulkanDevice->createBuffer(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            instanceBuffer.size,
            &stagingBuffer.buffer,
            &stagingBuffer.memory,
            instanceData.data()));

        VK_CHECK_RESULT(vulkanDevice->createBuffer(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            instanceBuffer.size,
            &instanceBuffer.buffer,
            &instanceBuffer.memory));

        // Copy to staging buffer
        VkCommandBuffer copyCmd = VulkanExampleBase::createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        VkBufferCopy copyRegion = { };
        copyRegion.size = instanceBuffer.size;
        vkCmdCopyBuffer(
            copyCmd,
            stagingBuffer.buffer,
            instanceBuffer.buffer,
            1,
            &copyRegion);

        VulkanExampleBase::flushCommandBuffer(copyCmd, queue, true);

        instanceBuffer.descriptor.range = instanceBuffer.size;
        instanceBuffer.descriptor.buffer = instanceBuffer.buffer;
        instanceBuffer.descriptor.offset = 0;

        // Destroy staging resources
        vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
        vkFreeMemory(device, stagingBuffer.memory, nullptr);
    }

    void prepareUniformBuffers()
    {
        VK_CHECK_RESULT(vulkanDevice->createBuffer(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &uniformBuffers.scene,
            sizeof(uboVS)));

        // Map persistent
        VK_CHECK_RESULT(uniformBuffers.scene.map());

        updateUniformBuffer(true);
    }

    void updateLight()
    {
        static float     G  = 2.5f;
        static float     mi = 10.0f;
        static float     mp = 100.0f;
        static glm::vec3 pi = { 45.0f, 0.0f, 10.0f };
        static glm::vec3 pp = { 0.0f,  0.0f, 0.0f };
        static glm::vec3 vi = { -1.0f, -0.3f, 1.0f };
        static glm::vec3 ai = { 0.0f,  0.0f, 0.0f };
        static glm::vec3 fi = { 0.0f,  0.0f, 0.0f };

        float r = glm::distance(pi, pp);
        glm::vec3 dirVec = glm::normalize(pi - pp);
        fi = -dirVec * G*mp*mi/(r*r);
        ai = fi/mi;
        vi = vi + ai*frameTimer;
        pi = pi + vi*frameTimer;

        const float k = 0.25f * frameTimer;
        uboVS.lightInt = LIGHT_INTENSITY*k + uboVS.lightInt*(1.0f - k);
        uboVS.lightPos = glm::vec4(pi, 1.0f);
    }

    void updateUniformBuffer(bool viewChanged)
    {
        if (viewChanged)
        {
            std::cout << "  >> VulkanExample-229::updateUniformBuffer(bool viewChanged) cameraPos = {" << cameraPos.x << " , " << cameraPos.y << " , " << cameraPos.z << "}\n";
            std::cout << "  >> VulkanExample-229::updateUniformBuffer(bool viewChanged) rotation = {" << rotation.x << ",  " << rotation.y << " , " << rotation.z << "}\n";
            std::cout << "  >> VulkanExample-229::updateUniformBuffer(bool viewChanged) zoom = {" << zoom << "}\n";

            uboVS.projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 256.0f);

            uboVS.view = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, zoom)) * glm::translate(glm::mat4(), cameraPos);
            uboVS.view = glm::rotate(uboVS.view, glm::radians(rotation.x/16), glm::vec3(1.0f, 0.0f, 0.0f));
            uboVS.view = glm::rotate(uboVS.view, glm::radians(rotation.y/16), glm::vec3(0.0f, 1.0f, 0.0f));
            uboVS.view = glm::rotate(uboVS.view, glm::radians(rotation.z/16), glm::vec3(0.0f, 0.0f, 1.0f));

            // Computing REAL camera coordinates, with rotation, zoom, etc... from MV matrix.
            glm::mat3 rotMat(uboVS.view);
            glm::vec3 d(uboVS.view[3]);
            uboVS.camPos = glm::vec4(-d * rotMat, 1.0f);
        }

        if (!paused)
        {
            uboVS.locSpeed  += frameTimer * 0.35f;
            uboVS.globSpeed += frameTimer * 0.01f;
            updateLight();
        }
        memcpy(uniformBuffers.scene.mapped, &uboVS, sizeof(uboVS));
    }

    void draw()
    {
        VulkanExampleBase::prepareFrame();

        // Command buffer to be sumitted to the queue
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

        // Submit to queue
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

        VulkanExampleBase::submitFrame();
    }

    void prepare()
    {
        VulkanExampleBase::prepare();
        loadAssets();
        prepareInstanceData();
        prepareUniformBuffers();
        setupDescriptorSetLayout();
        preparePipelines();
        setupDescriptorPool();
        setupDescriptorSet();
        buildCommandBuffers();
        prepared = true;
    }

    virtual void render()
    {
        if (!prepared)
        {
            return;
        }
        draw();
        if (!paused)
        {
            updateUniformBuffer(false);
        }
    }

    virtual void viewChanged()
    {
        updateUniformBuffer(true);
    }

    virtual void getOverlayText(VulkanTextOverlay *textOverlay)
    {
        textOverlay->addText("Rendering " + std::to_string(INSTANCE_COUNT) + " instances", 5.0f, 85.0f, VulkanTextOverlay::alignLeft);
    }
};

VULKAN_EXAMPLE_MAIN()
