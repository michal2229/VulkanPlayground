/*
* Vulkan Example - rendering a scene made of several objects.
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
#define DESCRIPTOR_COUNT        1     // Basically number of models here.
#define ENABLE_VALIDATION       false


/////////////////////////////////////////
/// Model specific objects:
/// * texture
/// * model
/// * vert shader
/// * frag shader
/// * pipeline
/// * descriptor set
/// * commands in command buffer
/////////////////////////////////////////


class VulkanExample : public VulkanExampleBase
{
public:
    // M V P
    // M - MODEL MAT      - model space -> world space
    // V - VIEW MAT       - world space -> camera space
    // P - PROJECTION MAT - camera space -> square frustum space
    // MVP = P * V * M
    // gl_Position =  MVP * vec4(inPos, 1.0f);
    // someVector  =  MVP * vec4(inVec, 0.0f);
    struct UBOVS {
        // model matrix is not used here because verts are already moved in *.obj files
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 camPos   = glm::vec4();
    } uboVS;

    // Scene buffer.
    struct {
        vks::Buffer scene;
    } uniformBuffers;

    // Vertex layout for the models.
    vks::VertexLayout vertexLayout = vks::VertexLayout({
        vks::VERTEX_COMPONENT_POSITION,
        vks::VERTEX_COMPONENT_NORMAL,
        vks::VERTEX_COMPONENT_UV,
        vks::VERTEX_COMPONENT_COLOR,
    });

    // Pipeline layout.
    VkPipelineLayout pipelineLayout;


    // Textures.
    struct {
        vks::Texture2D all_ao_tex2D;
    } textures;

    // Models.
    struct {
        vks::Model monkey_model;
    } models;

    // Pipelines.
    struct {
        VkPipeline monkey_VkPipeline;
    } pipelines;

    // Descriptor sets.
    VkDescriptorSetLayout descriptorSetLayout;
    struct {
        VkDescriptorSet monkey_VkDescriptorSet;
    } descriptorSets;


    VulkanExample() : VulkanExampleBase(ENABLE_VALIDATION)
    {
        title = "Vulkan Example - a scene";
        enableTextOverlay = true;
        srand(time(NULL));
        cameraPos = { 0.0f, 0.0f, 0.0f };
        rotation = {-45.0f, 180.0f, 0.0f };
        zoom = -10.0f;
        rotationSpeed = 0.25f;
        camera.setPerspective(80.0f, (float)width / (float)height, 0.1f, 1024.0f);
    }

    ~VulkanExample()
    {
        vkDestroyPipeline(device, pipelines.monkey_VkPipeline, nullptr);

        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

        models.monkey_model.destroy();

        textures.all_ao_tex2D.destroy();

        uniformBuffers.scene.destroy();
    }

    void buildCommandBuffers() override
    {
        VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

        VkClearValue clearValues[2];
        clearValues[0].color = { { 0.8f, 0.9f, 1.0f, 0.0f } };
        clearValues[1].depthStencil = { 1.0f, 0u };

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

            { // Monkey commands into command buffer
                vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.monkey_VkDescriptorSet, 0, NULL);
                vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.monkey_VkPipeline);
                vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &models.monkey_model.vertices.buffer, offsets);
                vkCmdBindIndexBuffer(drawCmdBuffers[i], models.monkey_model.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(drawCmdBuffers[i], models.monkey_model.indexCount, 1, 0, 0, 0);
            }

            vkCmdEndRenderPass(drawCmdBuffers[i]);

            VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
        }
    }

    void loadAssets()
    {
        { // Monkey model loading from file
            models.monkey_model.loadFromFile(getAssetPath() + "models/my_new_scene1/monkey.obj",    vertexLayout, 1.0f,   vulkanDevice, queue);
        }

        // Textures
        // Get supported compressed texture format
        if (!vulkanDevice->features.textureCompressionBC)
        {
            vks::tools::exitFatal("Device does not support VK_FORMAT_BC3_UNORM_BLOCK compressed texture format!", "Error");
        }

        { // AO texture loading from file
            textures.all_ao_tex2D.loadFromFile(getAssetPath()   + "textures/my_new_scene1/all_ao.dds", VK_FORMAT_BC3_UNORM_BLOCK, vulkanDevice, queue);
        }
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

        { // Monkey descriptor set
            VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descripotrSetAllocInfo, &descriptorSets.monkey_VkDescriptorSet));
            writeDescriptorSets = {
                vks::initializers::writeDescriptorSet(descriptorSets.monkey_VkDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &uniformBuffers.scene.descriptor),			// Binding 0 : Vertex shader uniform buffer
                vks::initializers::writeDescriptorSet(descriptorSets.monkey_VkDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textures.all_ao_tex2D.descriptor)	// Binding 1 : AO map
            };
            vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
        }
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

        // This example uses one input state - for non-instanced rendering
        VkPipelineVertexInputStateCreateInfo inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
        std::vector<VkVertexInputBindingDescription>  bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        // Vertex input bindings
        bindingDescriptions = {
            // Binding point 0: Mesh vertex layout description at per-vertex rate
            vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, vertexLayout.stride(), VK_VERTEX_INPUT_RATE_VERTEX),
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
        };
        inputState.pVertexBindingDescriptions = bindingDescriptions.data();
        inputState.pVertexAttributeDescriptions = attributeDescriptions.data();

        pipelineCreateInfo.pVertexInputState = &inputState;

        { // Monkey shaders loading from fpv files
            shaderStages[0] = loadShader(getAssetPath() + "shaders/my_new_scene1/default_transforms.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
            shaderStages[1] = loadShader(getAssetPath() + "shaders/my_new_scene1/default_material.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
            // Only use the non-instanced input bindings and attribute descriptions
            inputState.vertexBindingDescriptionCount = 1;
            inputState.vertexAttributeDescriptionCount = 4;
            VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.monkey_VkPipeline));
        }
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

    void updateUniformBuffer(bool viewChanged)
    {
        if (viewChanged)
        {
            uboVS.projection = camera.matrices.perspective;

            uboVS.view = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, zoom)) * glm::translate(glm::mat4(), cameraPos);
            uboVS.view = glm::rotate(uboVS.view, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            uboVS.view = glm::rotate(uboVS.view, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            uboVS.view = glm::rotate(uboVS.view, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            // Computing camera coordinates, with rotation, zoom, etc, from MV matrix.
            glm::mat3 rotMat(uboVS.view);
            glm::vec3 d(uboVS.view[3]);
            uboVS.camPos = glm::vec4(-d * rotMat, 1.0f);
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

    void prepare() override
    {
        VulkanExampleBase::prepare();
        loadAssets();
        prepareUniformBuffers();
        setupDescriptorSetLayout();
        preparePipelines();
        setupDescriptorPool();
        setupDescriptorSet();
        buildCommandBuffers();
        prepared = true;
    }

    virtual void render() override
    {
        // Fix instability when freezing runtime or low fps.
        if (frameTimer > 0.1f) frameTimer = 0.1f;

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

    virtual void viewChanged() override
    {
        updateUniformBuffer(true);
    }

    virtual void getOverlayText(VulkanTextOverlay *textOverlay) override
    {
        textOverlay->addText("LMB to rotate, MMB to move, RMB or numpad +/- to zoom", 5.0f, 105.0f, VulkanTextOverlay::alignLeft);
    }

    virtual void keyPressed(uint32_t key) override
    {
        switch (key)
        {
        case KEY_KPADD:
            zoom /= 1.41f;
            updateUniformBuffer(true);
        break;
        case KEY_KPSUB:
            zoom *= 1.41f;
            updateUniformBuffer(true);
        break;
        }
    }
};

VULKAN_EXAMPLE_MAIN()
