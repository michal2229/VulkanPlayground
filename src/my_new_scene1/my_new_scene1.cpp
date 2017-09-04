/*
* Vulkan Example - rendering a scene made of several objects.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <vector>
#include <map>
#include <random>
#include <3DEntity.hpp>

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
        vks::VERTEX_COMPONENT_TANGENT,
        vks::VERTEX_COMPONENT_BITANGENT,
        vks::VERTEX_COMPONENT_UV,
        vks::VERTEX_COMPONENT_COLOR,
    });

    // Pipeline layout.
    VkPipelineLayout pipelineLayout;

    // Descriptor set layout
    VkDescriptorSetLayout descriptorSetLayout;

    // NEW APPROACH
    std::map<std::string, vks::Texture2D>  texturesMap;
    std::map<std::string, vks::Model>      modelsMap;
    std::map<std::string, VkPipeline>      pipelinesMap;
    std::map<std::string, VkDescriptorSet> descriptorSetsMap;

    // Scene definition here.
    std::map<std::string, std::string> usedTextureSetInfoMap = {
        {"1COLOR",       "all_diffuse_C.dds"},
        {"2DIFFUSE_DI",  "all_diffuse_DI.dds"},
        {"3AO",          "all_ao.dds"},
        {"4EMIT",        "all_emit.dds"},
        {"5NORMAL",      "all_normal.dds"},
        {"6REFLECTION",  "reflection_center.dds"},
    };
    std::map<std::string, std::string> usedShadersSetInfoMap = {
        {"VERT", "default_transforms.vert.spv"},
        {"FRAG", "default_material.frag.spv"},
    };
    std::vector<Entity3dCreateInfo> entities3dCreateInfoVec = {
        {"box",    "box.obj",   usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"light",  "light.obj", usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"floor",  "floor.obj", usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"cube1",  "cube1.obj", usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"cube2",  "cube2.obj", usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"cube3",  "cube3.obj", usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"monkey", "monkey.obj",usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"s1",     "s1.obj",    usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"s2",     "s2.obj",    usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"s3",     "s3.obj",    usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"s4",     "s4.obj",    usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"s5",     "s5.obj",    usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"s6",     "s6.obj",    usedTextureSetInfoMap, usedShadersSetInfoMap},
        {"full_droid", "full_droid.obj", usedTextureSetInfoMap, usedShadersSetInfoMap},
    };
    std::map<std::string, Entity3dTraitsSet> entities3dTraitsMap;


    VulkanExample() : VulkanExampleBase(ENABLE_VALIDATION)
    {
        title = "Vulkan Example - a scene";
        enableTextOverlay = true;
        srand(time(NULL));
        cameraPos = { 0.0f, 0.0f, 0.0f };
        rotation = {-45.0f, 25.0f, 0.0f };
        zoom = -10.0f;
        rotationSpeed = 0.25f;
        camera.setPerspective(80.0f, (float)width / (float)height, 0.1f, 1024.0f);

        // INIT
        this->initVulkan();
        this->setupWindow();
        this->initSwapchain();
        this->prepare();
    }

    ~VulkanExample()
    {
        for (auto& pipM : pipelinesMap)
        {
            vkDestroyPipeline(device, pipM.second, nullptr);
        }

        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

        for (auto& modM : modelsMap)
        {
            modM.second.destroy();
        }

        for (auto& texM : texturesMap)
        {
            texM.second.destroy();
        }

        uniformBuffers.scene.destroy();
    }

    void buildEntity3dTraitsSets()
    {
        for (auto& ent3dCreInf : entities3dCreateInfoVec)
        {
            auto& entName = ent3dCreInf.entityName;
            auto& ent = entities3dTraitsMap[entName];

            for (auto& texInfoMap : ent3dCreInf.textureMap)
            {
                auto& texType = texInfoMap.first;
                auto& texName = texInfoMap.second;

                ent.texturePtrMap[texType] = &texturesMap[texName];
            }

            ent.modelPtr           = &modelsMap[entName];
            ent.vkPipelinePtr      = &pipelinesMap[entName];
            ent.vkDescriptorSetPtr = &descriptorSetsMap[entName];
        }

        assert(entities3dCreateInfoVec.size() == entities3dTraitsMap.size());
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

            { // All 3d objects commands into buffer
                for (auto& traitMap : entities3dTraitsMap)
                {
                    Entity3dTraitsSet& obj = traitMap.second;
                    vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, obj.vkDescriptorSetPtr, 0, NULL);
                    vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *obj.vkPipelinePtr);
                    vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &(obj.modelPtr -> vertices.buffer), offsets);
                    vkCmdBindIndexBuffer(drawCmdBuffers[i], obj.modelPtr -> indices.buffer, 0, VK_INDEX_TYPE_UINT32);
                    vkCmdDrawIndexed(drawCmdBuffers[i], obj.modelPtr -> indexCount, 1, 0, 0, 0);
                }

            }

            vkCmdEndRenderPass(drawCmdBuffers[i]);

            VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
        }
    }

    void loadAssets()
    {
        // Textures
        // Get supported compressed texture format
        if (!vulkanDevice->features.textureCompressionBC)
        {
            vks::tools::exitFatal("Device does not support VK_FORMAT_BC3_UNORM_BLOCK compressed texture format!", "Error");
        }

        { // All textures
            for (auto& ent3dCreInf : entities3dCreateInfoVec)
            {
                for (auto& texInfoMap : ent3dCreInf.textureMap)
                {
                    auto& texName = texInfoMap.second;

                    vks::Texture2D tex;
                    tex.loadFromFile(getAssetPath() + "textures/my_new_scene1/"+texName, VK_FORMAT_BC3_UNORM_BLOCK, vulkanDevice, queue);
                    texturesMap[texName] = std::move(tex);
                }
            }
        }

        { // All models
            for (auto& ent3dCreInf : entities3dCreateInfoVec)
            {
                auto& entityName = ent3dCreInf.entityName;
                if (modelsMap.find(entityName) == modelsMap.end())
                {
                    auto& modelName = ent3dCreInf.modelName;
                    vks::Model model;
                    model.loadFromFile(getAssetPath() + "models/my_new_scene1/"+modelName, vertexLayout, 1.0f, vulkanDevice, queue);
                    modelsMap[entityName] = std::move(model);
                }
            }
        }
    }

    void setupDescriptorPool()
    {
        const uint32_t descriptorCount = entities3dCreateInfoVec.size();

        // Example uses one ubo
        std::vector<VkDescriptorPoolSize> poolSizes =
        {
            vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorCount),
        };

        for (int i = 0; i < usedTextureSetInfoMap.size(); i++)
        {
            poolSizes.push_back(
                        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCount)
                    );
        }

        VkDescriptorPoolCreateInfo descriptorPoolInfo =
            vks::initializers::descriptorPoolCreateInfo(
                poolSizes.size(),
                poolSizes.data(),
                descriptorCount);

        VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
    }

    void setupDescriptorSetLayout()
    {
        uint32_t bindId = 0u;

        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
        setLayoutBindings.push_back(
                    // Binding 0 : Vertex shader uniform buffer
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT,
                        bindId++)
                    );

        // Putting samplers for all types of textures used in this scene
        for (int i = 0; i < usedTextureSetInfoMap.size(); i++)
        {
            std::cout << " >>> setupDescriptorSetLayout: adding bind of id: " << bindId << "\n";
            setLayoutBindings.push_back(
                // Binding: Fragment shader combined sampler
                vks::initializers::descriptorSetLayoutBinding(
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    bindId++)
                        );

        }

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

        { // All models descriptor set setup
            for (auto& ent3dCreInf : entities3dCreateInfoVec)
            {
                auto& entityName = ent3dCreInf.entityName;
                if (descriptorSetsMap.find(entityName) == descriptorSetsMap.end())
                {
                    VkDescriptorSet descSet;

                    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descripotrSetAllocInfo, &descSet));
                    writeDescriptorSets = {
                        vks::initializers::writeDescriptorSet(descSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &uniformBuffers.scene.descriptor),			// Binding 0 : Vertex shader uniform buffer
                            // Binding 1 : AO map
                    };
                    for (auto& usedTexInfoM : usedTextureSetInfoMap)
                    {
                        auto& texType = usedTexInfoM.first;
                        std::cout << " >>> setupDescriptorSet: adding " << writeDescriptorSets.size() << " " << texType << " for " << entityName << "\n";
                        writeDescriptorSets.push_back(
                            // Binding 1 : Fragment shader combined sampler
                            vks::initializers::writeDescriptorSet(descSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, writeDescriptorSets.size(), &texturesMap[ent3dCreInf.textureMap[texType]].descriptor)
                        );

                    }

                    vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);

                    descriptorSetsMap[entityName] = std::move(descSet);
                }
            }
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
            vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),                     // Location 0: Position
            vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),     // Location 1: Normal
            vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 6),     // Location 2: Tangent
            vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 9),     // Location 3: Bitangent
            vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 4, VK_FORMAT_R32G32_SFLOAT,    sizeof(float) * 12),    // Location 4: Texture coordinates
            vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 5, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 14),    // Location 5: Color
        };
        inputState.pVertexBindingDescriptions = bindingDescriptions.data();
        inputState.pVertexAttributeDescriptions = attributeDescriptions.data();

        pipelineCreateInfo.pVertexInputState = &inputState;

        { // All models pipeline creation
            for (auto& ent3dCreInf : entities3dCreateInfoVec)
            {
                auto& entityName = ent3dCreInf.entityName;
                if (pipelinesMap.find(entityName) == pipelinesMap.end())
                {
                    VkPipeline pip;

                    shaderStages[0] = loadShader(getAssetPath() + "shaders/my_new_scene1/"+ent3dCreInf.shadersMap["VERT"], VK_SHADER_STAGE_VERTEX_BIT);
                    shaderStages[1] = loadShader(getAssetPath() + "shaders/my_new_scene1/"+ent3dCreInf.shadersMap["FRAG"], VK_SHADER_STAGE_FRAGMENT_BIT);
                    // Only use the non-instanced input bindings and attribute descriptions
                    inputState.vertexBindingDescriptionCount = 1; // Number of bingins (ubo etc.)
                    inputState.vertexAttributeDescriptionCount = attributeDescriptions.size();
                    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pip));

                    pipelinesMap[entityName] = std::move(pip);
                }
            }
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
        buildEntity3dTraitsSets();
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

// { MAIN
VulkanExample *vulkanExample;
static void handleEvent(const xcb_generic_event_t *event)
{
    if (vulkanExample != NULL)
    {
        vulkanExample->handleEvent(event);
    }
}

int main(const int argc, const char *argv[])
{
    for (size_t i = 0; i < argc; i++) { VulkanExample::args.push_back(argv[i]); };

    vulkanExample = new VulkanExample();

    vulkanExample->renderLoop();

    delete(vulkanExample);

    return 0;
}
// } // MAIN
