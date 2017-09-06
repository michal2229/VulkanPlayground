#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <map>
#include <VulkanTexture.hpp>
#include <VulkanModel.hpp>

namespace vk229
{
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

using entity_name_t  = std::string;
using texture_name_t = std::string;
using texture_type_t = std::string;
using texture_form_t = VkFormat;
using model_name_t   = std::string;
using shader_name_t  = std::string;
using shader_stage_t = VkShaderStageFlagBits;
using textures_set_name_t = std::string;
using shaders_set_name_t  = std::string;

VkPipelineShaderStageCreateInfo loadShader(VkDevice& dev, std::string fileName, VkShaderStageFlagBits stage, std::vector<VkShaderModule>& shaderModules)
{
        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = stage;
#if defined(__ANDROID__)
        shaderStage.module = vks::tools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device);
#else
        shaderStage.module = vks::tools::loadShader(fileName.c_str(), dev);
#endif
        shaderStage.pName = "main"; // todo : make param
        assert(shaderStage.module != VK_NULL_HANDLE);
        shaderModules.push_back(shaderStage.module);

        return shaderStage;
}

//enum class TextureType
//{
//    COLOR,
//    DIFFUSE_DI,
//    AO,
//    EMIT,
//    NORMAL,
//    REFLECTION
//};

struct UniformBufferVS {
    glm::mat4 view;
    glm::mat4 projection;
//    glm::vec4 camPos;
};

struct DeviceSideBuffers {
    vks::Buffer scene; // Scene buffer - device's side mapped memory.
};

// Texture info
struct TextureInfo
{
    texture_name_t textureName;
    texture_form_t textureFormat; // Compression etc.
};

struct ModelInfo
{
    model_name_t modelName;
    // TODO: transform matrix - part of one big dynamic UBO.
    // TODO: parent/child ptr - to apply parent's transforms to a child.
};

struct ShaderInfo
{
    shader_name_t  shaderName;
    shader_stage_t shaderType; // VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT...
};


struct TextureSetInfo
{
//    std::string setName;
    std::map<texture_type_t, TextureInfo> texturesInfoMap; // By tex type;
};

struct ShaderSetInfo
{
//    std::string setName;
    std::map<shader_name_t, ShaderInfo> shadersInfoMap; // By shader type;
};


// Element of scene definition.
struct Entity3dInfo
{
//    std::string entityName;
    ModelInfo           modelInfo;
    textures_set_name_t texturesSetName;
    shaders_set_name_t  shadersSetName;
};

// Used for init.
struct SceneInfo
{
    // Vertex layout for the models.
    vks::VertexLayout vertexLayout;

    std::map<textures_set_name_t, TextureSetInfo> texturesSetInfoMap;
    std::map<shaders_set_name_t,  ShaderSetInfo>  shadersSetInfoMap;
    std::map<model_name_t,  ModelInfo>           modelInfoMap;
    std::map<entity_name_t, Entity3dInfo>        entities3dInfoMap;

    SceneInfo() :
        vertexLayout({
            vks::VERTEX_COMPONENT_POSITION,
            vks::VERTEX_COMPONENT_NORMAL,
            vks::VERTEX_COMPONENT_TANGENT,
            vks::VERTEX_COMPONENT_BITANGENT,
            vks::VERTEX_COMPONENT_UV,
            vks::VERTEX_COMPONENT_COLOR,
        })
    {
    }

    uint32_t getTextureSetSize() const
    {
        return this->texturesSetInfoMap.begin()->second.texturesInfoMap.size();
    }

    uint32_t getNeededDescriptorCount() const
    {
        return this->entities3dInfoMap.size();
    }
};

// Used to store assets data.
struct SceneData
{
    VkPipelineLayout      pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    SceneInfo sceneInfo;

    UniformBufferVS uboVS;

    DeviceSideBuffers uniformBuffers;

    std::map<texture_name_t, vks::Texture2D>  texturesMap;       // By texture name
    std::map<model_name_t,   vks::Model>      modelsMap;         // By model name
    std::map<entity_name_t,  VkPipeline>      pipelinesMap;      // By entity name;
    std::map<entity_name_t,  VkDescriptorSet> descriptorSetsMap; // By entity name;

    SceneData()
    {
    }

    ~SceneData()
    {
    }

    void destroy(VkDevice& dev)
    {
        for (auto& pipM : this->pipelinesMap)
        {
            vkDestroyPipeline(dev, pipM.second, nullptr);
        }

        vkDestroyPipelineLayout(dev, this->pipelineLayout, nullptr);

        vkDestroyDescriptorSetLayout(dev, this->descriptorSetLayout, nullptr);

        for (auto& modM : this->modelsMap)
        {
            modM.second.destroy();
        }

        for (auto& texM : this->texturesMap)
        {
            texM.second.destroy();
        }

        this->uniformBuffers.scene.destroy();
    }

    bool isModelAlreadyCreated(model_name_t _mod) const
    {
        return this->modelsMap.find(_mod) != this->modelsMap.end();
    }

    bool isTextureAlreadyCreated(texture_name_t _tex) const
    {
        return this->texturesMap.find(_tex) != this->texturesMap.end();
    }

    bool isPipelineAlreadyCreated(entity_name_t _ent) const
    {
        return this->pipelinesMap.find(_ent) != this->pipelinesMap.end();
    }

    bool isDescriptorSetAlreadyCreated(entity_name_t _ds) const
    {
        return this->descriptorSetsMap.find(_ds) != this->descriptorSetsMap.end();
    }

    void copyDataToDeviceMemory()
    {
        memcpy(this->uniformBuffers.scene.mapped, &this->uboVS, sizeof(this->uboVS));
    }

    void buildCommandBuffer(VkCommandBuffer& drawCmdBuffer, uint32_t vertexBufferBindId, const VkDeviceSize* offsets)
    {
        for (auto& entCreInfMap : this->sceneInfo.entities3dInfoMap)
        {
            entity_name_t entName = entCreInfMap.first;
            auto& entCreInf     = entCreInfMap.second;

            model_name_t& modelName = entCreInf.modelInfo.modelName;

            auto& descrSet = this->descriptorSetsMap[entName];
            auto& pipeline = this->pipelinesMap[entName];
            auto& model    = this->modelsMap[modelName];

            vkCmdBindDescriptorSets(drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineLayout, 0, 1, &descrSet, 0, NULL);
            vkCmdBindPipeline(drawCmdBuffer,       VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdBindVertexBuffers(drawCmdBuffer,  vertexBufferBindId, 1, &(model.vertices.buffer), offsets);
            vkCmdBindIndexBuffer(drawCmdBuffer,    model.indices.buffer,  0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(drawCmdBuffer,        model.indexCount,      1, 0, 0, 0);
        }
    }

    void loadTextures(vks::VulkanDevice* dev, VkQueue& queue, std::string assetsPath)
    {
        auto& entities3dInfo = this->sceneInfo.entities3dInfoMap;
        for (auto& ent3dCreInf : this->sceneInfo.entities3dInfoMap) // <entity_name, Entity3dInfo>
        {
            textures_set_name_t& textureSetName = ent3dCreInf.second.texturesSetName;
            auto& textureSetInfo = this->sceneInfo.texturesSetInfoMap[textureSetName].texturesInfoMap;
            for (auto& texInfoMap : textureSetInfo)
            {
                auto& texInfo = texInfoMap.second;
                texture_name_t& texName = texInfo.textureName;
                texture_form_t& texFormat = texInfo.textureFormat;

                if (false == this->isTextureAlreadyCreated(texName))
                {

//                    // Get supported compressed texture format
//                    if (!vulkanDevice->features.textureCompressionBC)
//                    {
//                        vks::tools::exitFatal("Device does not support needed compressed texture format!", "Error");
//                    }

                    vks::Texture2D tex;
                    tex.loadFromFile(assetsPath + "textures/my_new_scene1/"+texName, texFormat, dev, queue);
                    this->texturesMap[texName] = std::move(tex);
                }
            }
        }
    }

    void loadModels(vks::VulkanDevice* dev, VkQueue& queue, std::string assetsPath)
    {
        auto& entities3dInfo = this->sceneInfo.entities3dInfoMap;
        for (auto& ent3dCreInf : entities3dInfo)
        {
            entity_name_t entityName = ent3dCreInf.first;
            auto& modelInfo  = ent3dCreInf.second.modelInfo;
            model_name_t& modelName = modelInfo.modelName;
            if (false == this->isModelAlreadyCreated(modelName))
            {
                vks::Model model;
                model.loadFromFile(assetsPath + "models/my_new_scene1/"+modelName, this->sceneInfo.vertexLayout, 1.0f, dev, queue);
                this->modelsMap[modelName] = std::move(model);
            }
        }

    }

    void setupDescriptorSetLayout(vks::VulkanDevice* dev)
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
        for (int i = 0; i < this->sceneInfo.getTextureSetSize(); i++)
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

        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(dev->logicalDevice, &descriptorLayout, nullptr, &this->descriptorSetLayout));

        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo(
                &this->descriptorSetLayout,
                1);

        VK_CHECK_RESULT(vkCreatePipelineLayout(dev->logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &this->pipelineLayout));
    }

    void setupDescriptorPool(vks::VulkanDevice* dev, VkDescriptorPool& descPool)
    {
        const uint32_t descriptorCount = this->sceneInfo.getNeededDescriptorCount();

        // Example uses one ubo
        std::vector<VkDescriptorPoolSize> poolSizes =
        {
            vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorCount),
        };

        for (int i = 0; i < this->sceneInfo.getTextureSetSize(); i++)
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

        VK_CHECK_RESULT(vkCreateDescriptorPool(dev->logicalDevice, &descriptorPoolInfo, nullptr, &descPool));
    }

    void setupDescriptorSet(vks::VulkanDevice* dev, VkDescriptorPool& descPool)
    {
        VkDescriptorSetAllocateInfo descripotrSetAllocInfo;
        std::vector<VkWriteDescriptorSet> writeDescriptorSets;

        descripotrSetAllocInfo = vks::initializers::descriptorSetAllocateInfo(descPool, &this->descriptorSetLayout, 1);


        auto& entities3dInfoMap = this->sceneInfo.entities3dInfoMap;
        for (auto& ent3dCreInf : entities3dInfoMap) // For all 3D entities.
        {
            entity_name_t entityName   = ent3dCreInf.first;
            vk229::Entity3dInfo& entity3dInfo = ent3dCreInf.second;

            if (false == this->isDescriptorSetAlreadyCreated(entityName)) // If not already created.
            {
                VkDescriptorSet descSet;

                VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->logicalDevice, &descripotrSetAllocInfo, &descSet));
                writeDescriptorSets = {
                    vks::initializers::writeDescriptorSet(descSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &this->uniformBuffers.scene.descriptor), // Binding 0 : Vertex shader uniform buffer
                };

                textures_set_name_t& texSetName = entity3dInfo.texturesSetName;
                TextureSetInfo& texSetInfo = this->sceneInfo.texturesSetInfoMap[texSetName];
                auto& texSetInfoMap = texSetInfo.texturesInfoMap;

                for (auto& usedTexInfo : texSetInfoMap)
                {
                    texture_type_t texType = usedTexInfo.first;
                    texture_name_t texName = usedTexInfo.second.textureName;
                    auto& textureDescriptor = this->texturesMap[texName].descriptor;
                    std::cout << " >>> setupDescriptorSet: adding " << writeDescriptorSets.size() << " " << texType << " for " << entityName << "\n";
                    writeDescriptorSets.push_back(
                        // Binding i : Fragment shader combined sampler - for every texture
                        vks::initializers::writeDescriptorSet(descSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, writeDescriptorSets.size(), &textureDescriptor)
                    );
                }

                vkUpdateDescriptorSets(dev->logicalDevice, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);

                this->descriptorSetsMap[entityName] = std::move(descSet);
            }
        }
    }

    void preparePipelines(vks::VulkanDevice* dev, VkRenderPass renderPass, VkPipelineCache pipelineCache, uint32_t vertedBindId, std::string assetsPath, std::vector<VkShaderModule> shaderModules)
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
                this->pipelineLayout,
                renderPass,
                0);

        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineCreateInfo.pRasterizationState = &rasterizationState;
        pipelineCreateInfo.pColorBlendState    = &colorBlendState;
        pipelineCreateInfo.pMultisampleState   = &multisampleState;
        pipelineCreateInfo.pViewportState      = &viewportState;
        pipelineCreateInfo.pDepthStencilState  = &depthStencilState;
        pipelineCreateInfo.pDynamicState       = &dynamicState;
        pipelineCreateInfo.stageCount          = shaderStages.size();
        pipelineCreateInfo.pStages             = shaderStages.data();

        // This example uses one input state - for non-instanced rendering
        VkPipelineVertexInputStateCreateInfo inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
        std::vector<VkVertexInputBindingDescription>  bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        // Vertex input bindings
        bindingDescriptions = {
            // Binding point 0: Mesh vertex layout description at per-vertex rate
            vks::initializers::vertexInputBindingDescription(vertedBindId, this->sceneInfo.vertexLayout.stride(), VK_VERTEX_INPUT_RATE_VERTEX),
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
            vks::initializers::vertexInputAttributeDescription(vertedBindId, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),                     // Location 0: Position
            vks::initializers::vertexInputAttributeDescription(vertedBindId, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),     // Location 1: Normal
            vks::initializers::vertexInputAttributeDescription(vertedBindId, 2, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 6),     // Location 2: Tangent
            vks::initializers::vertexInputAttributeDescription(vertedBindId, 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 9),     // Location 3: Bitangent
            vks::initializers::vertexInputAttributeDescription(vertedBindId, 4, VK_FORMAT_R32G32_SFLOAT,    sizeof(float) * 12),    // Location 4: Texture coordinates
            vks::initializers::vertexInputAttributeDescription(vertedBindId, 5, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 14),    // Location 5: Color
        };
        inputState.pVertexBindingDescriptions = bindingDescriptions.data();
        inputState.pVertexAttributeDescriptions = attributeDescriptions.data();

        pipelineCreateInfo.pVertexInputState = &inputState;

        { // All models pipeline creation
            for (auto& ent3dCreInf : this->sceneInfo.entities3dInfoMap)
            {
                entity_name_t entityName = ent3dCreInf.first;
                Entity3dInfo& entity3dInfo = ent3dCreInf.second;
                if (false == this->isPipelineAlreadyCreated(entityName))
                {
                    VkPipeline pip;

                    shaders_set_name_t& shadSetName = entity3dInfo.shadersSetName;
                    ShaderSetInfo& shadSetInfo = this->sceneInfo.shadersSetInfoMap[shadSetName];
                    auto& shadSetInfoMap = shadSetInfo.shadersInfoMap;

                    auto& vertShaderInfo = shadSetInfoMap["VERT"];
                    auto& fragShaderInfo = shadSetInfoMap["FRAG"];

                    shaderStages[0] = loadShader(dev->logicalDevice, assetsPath + "shaders/my_new_scene1/" + vertShaderInfo.shaderName, vertShaderInfo.shaderType, shaderModules);
                    shaderStages[1] = loadShader(dev->logicalDevice, assetsPath + "shaders/my_new_scene1/" + fragShaderInfo.shaderName, fragShaderInfo.shaderType, shaderModules);
                    // Only use the non-instanced input bindings and attribute descriptions
                    inputState.vertexBindingDescriptionCount = 1; // Number of bingins (ubo etc.)
                    inputState.vertexAttributeDescriptionCount = attributeDescriptions.size();
                    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pip));

                    this->pipelinesMap[entityName] = std::move(pip);
                }
            }
        }
    }

    void prepareUniformBuffers(vks::VulkanDevice* dev, glm::mat4& viewMat, glm::mat4& perspMat)
    {
        VK_CHECK_RESULT(dev->createBuffer(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &this->uniformBuffers.scene,
            sizeof(this->uboVS)));

        // Map persistent
        VK_CHECK_RESULT(this->uniformBuffers.scene.map());

        this->updateUniformBuffers(true, viewMat, perspMat);
    }

    void updateUniformBuffers(bool viewChanged, glm::mat4& viewMat, glm::mat4& perspMat)
    {
        if (viewChanged)
        {
            this->uboVS.view       = viewMat;
            this->uboVS.projection = perspMat;
        }

        // Copy to device memory.
        this->copyDataToDeviceMemory();
    }
};


} // namespace vk229
