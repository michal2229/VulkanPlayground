#pragma once

#include <assert.h>
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

enum class TexT
{
    COLOR,
    DIFFUSE_DI,
    AO,
    EMIT,
    NORMAL,
    REFLECTION
};
std::map<TexT, std::string> TexTDesc
{
    {TexT::COLOR,      "COLOR"},
    {TexT::DIFFUSE_DI, "DIFFUSE_DI"},
    {TexT::AO,         "AO"},
    {TexT::EMIT,       "EMIT"},
    {TexT::NORMAL,     "NORMAL"},
    {TexT::REFLECTION, "REFLECTION"},
};
std::map<VkShaderStageFlagBits, std::string> ShadTDesc
{
    {VK_SHADER_STAGE_VERTEX_BIT,                    "VertS"},
    {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,      "TessCS"},
    {VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,   "TessES"},
    {VK_SHADER_STAGE_GEOMETRY_BIT,                  "GeomS"},
    {VK_SHADER_STAGE_FRAGMENT_BIT,                  "FragS"},
};


using texture_name_t     = std::string;
using texture_filename_t = std::string;
using texture_type_t     = TexT;
using texture_form_t     = VkFormat;

using model_name_t     = std::string;
using model_filename_t = std::string;

using shader_name_t      = std::string;
using shader_filename_t  = std::string;
using shader_stage_t     = VkShaderStageFlagBits;

using matrix_name_t    = std::string;
using matrix_content_t = glm::mat4x4;

using textures_set_name_t = std::string;
using shaders_set_name_t  = std::string;

using entity_name_t   = std::string;


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

struct UniformBufferVS {
    glm::mat4 view;
    glm::mat4 projection;
//    glm::vec4 camPos;
};

struct DeviceSideBuffers {
    vks::Buffer scene; // Scene buffer - device's side mapped memory.
};

//////////////////////////////////////
/// Information about texture.
/// Properties:
/// * texture_name
/// * texture_compression
/// * texture_type
/// * texture_filename
struct TextureInfo
{
    texture_name_t     textureName;
    texture_form_t     textureFormat; // Compression etc.
    texture_type_t     textureType;   // Is it normal map? Is it color?
    texture_filename_t textureFilename;
};

//////////////////////////////////////
/// Information about mesh.
/// Properties:
/// * model_name
/// * model_filename
struct ModelInfo
{
    model_name_t     modelName;
    model_filename_t modelFilename;
};

//////////////////////////////////////
/// Information about fingle shader.
/// Properties:
/// * shader_name
/// * shader_stage
/// * shader_filename
struct ShaderInfo
{
    shader_name_t     shaderName;
    shader_stage_t    shaderStage; // VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT...
    shader_filename_t shaderFilename;
};

//////////////////////////////////////
/// Information about textures set.
/// Properties:
/// * textures_set_name
/// * textures_names_vector
struct TextureSetInfo
{
    textures_set_name_t         texturesSetName;
    std::vector<texture_name_t> texturesNames;
//    std::map<texture_type_t, TextureInfo> texturesInfoMap; // By tex type;
};

//////////////////////////////////////
/// Information about model matrix.
/// Properties:
/// * matrix_name
/// * matrix_filename
struct MatrixInfo
{
    matrix_name_t    matrixName;
    matrix_content_t matrix;
};

//////////////////////////////////////
/// Information about shaders set.
/// Properties:
/// * shaders_set_name
/// * shaders_names_vector
struct ShaderSetInfo
{
    shaders_set_name_t         shadersSetName;
    std::vector<shader_name_t> shadersNames;
//    std::map<shader_stage_t, ShaderInfo> shadersInfoMap; // By shader type;
};

//////////////////////////////////////
/// Element of scene definition.
/// Properties:
/// * entity_name
/// * model_name
/// * textures_set_name
/// * shaders_set_name
/// * model_matrix_name
/// * parent_model_name - TODO
struct Entity3dInfo
{
    entity_name_t       entityName;
    model_name_t        modelName;
    matrix_name_t       matrixName;
    textures_set_name_t texturesSetName;
    shaders_set_name_t  shadersSetName;
    // TODO: transform matrix - part of one big dynamic UBO (or not).
    // TODO: parent/child ptr - to apply parent's transforms to a child.
};

// Used for init.
struct SceneInfo
{
    // Vertex layout for the models.
    vks::VertexLayout vertexLayout;

    std::map<model_name_t,  ModelInfo>      modelsInfoMap;
    std::map<shader_name_t, ShaderInfo>     shadersInfoMap;
    std::map<texture_name_t,  TextureInfo>  texturesInfoMap;
    std::map<texture_name_t,  MatrixInfo>   matriciesInfoMap;

    std::map<textures_set_name_t, TextureSetInfo> texturesSetInfoMap;
    std::map<shaders_set_name_t,  ShaderSetInfo>  shadersSetInfoMap;

    std::map<entity_name_t, Entity3dInfo>   entities3dInfoMap;


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

    void fillModelsInfoMap(const std::vector<ModelInfo>& modInfVec)
    {
        for (const ModelInfo& mi : modInfVec)
        {
            model_name_t mName = mi.modelName;
            this->modelsInfoMap[mName] = mi;
        }
    }

    void fillShadersInfoMap(const std::vector<ShaderInfo>& shadInfVec)
    {
        for (const ShaderInfo& si : shadInfVec)
        {
            shader_name_t sName = si.shaderName;
            this->shadersInfoMap[sName] = si;
        }
    }

    void fillTexturesInfoMap(const std::vector<TextureInfo>& texInfVec)
    {
        for (const TextureInfo& ti : texInfVec)
        {
            texture_name_t tName = ti.textureName;
            this->texturesInfoMap[tName] = ti;
        }
    }

    void fillMatricesInfoMap(const std::vector<MatrixInfo>& matInfVec)
    {
        for (const MatrixInfo& mi : matInfVec)
        {
            matrix_name_t mName = mi.matrixName;
            this->matriciesInfoMap[mName] = mi;
        }
    }

    void fillTexturesSetInfoMap(const std::vector<TextureSetInfo>& texSetInfVec)
    {
        for (const TextureSetInfo& tsi : texSetInfVec)
        {
            textures_set_name_t tsName = tsi.texturesSetName;
            this->texturesSetInfoMap[tsName] = tsi;
        }
    }

    void fillShadersSetInfoMap(const std::vector<ShaderSetInfo>& shadSetInfVec)
    {
        for (const ShaderSetInfo& ssi : shadSetInfVec)
        {
            shaders_set_name_t ssName = ssi.shadersSetName;
            this->shadersSetInfoMap[ssName] = ssi;
        }
    }

    void fillEntities3dInfoMap(const std::vector<Entity3dInfo>& entInfVec)
    {
        for (const Entity3dInfo& ei : entInfVec)
        {
            entity_name_t eName = ei.entityName;
            this->entities3dInfoMap[eName] = ei;
        }
    }

    uint32_t getTextureSetSize() const
    {
        return this->texturesSetInfoMap.begin()->second.texturesNames.size();
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

// HELPERS {

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

// } // HELPERS

// PREPARE {

    void loadTextures(vks::VulkanDevice* dev, VkQueue& queue, std::string assetsPath)
    {
        auto& entities3dInfo = this->sceneInfo.entities3dInfoMap;
        for (auto& ent3dCreInf : entities3dInfo) // <entity_name, Entity3dInfo>
        {
            textures_set_name_t& textureSetName   = ent3dCreInf.second.texturesSetName;
            vk229::TextureSetInfo& textureSetInfo = this->sceneInfo.texturesSetInfoMap[textureSetName];
            for (const vk229::texture_name_t& texName : textureSetInfo.texturesNames)
            {
                TextureInfo& texInfo          = this->sceneInfo.texturesInfoMap[texName];
                texture_form_t&     texFormat = texInfo.textureFormat;
                texture_filename_t& texFName  = texInfo.textureFilename;

                assert(texName == texInfo.textureName);

                if (false == this->isTextureAlreadyCreated(texName))
                {

//                    // Get supported compressed texture format
//                    if (!vulkanDevice->features.textureCompressionBC)
//                    {
//                        vks::tools::exitFatal("Device does not support needed compressed texture format!", "Error");
//                    }

                    vks::Texture2D tex;
                    tex.loadFromFile(assetsPath + "textures/my_new_scene1/"+texFName, texFormat, dev, queue);
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
//            entity_name_t entityName = ent3dCreInf.first;
            Entity3dInfo  entityInfo = ent3dCreInf.second;

            model_name_t modelName = entityInfo.modelName;
            ModelInfo    modelInfo = this->sceneInfo.modelsInfoMap[modelName];

            model_filename_t& modelFName = modelInfo.modelFilename;

            assert(modelName == modelInfo.modelName);

            if (false == this->isModelAlreadyCreated(modelName))
            {
                vks::Model model;
                model.loadFromFile(assetsPath + "models/my_new_scene1/"+modelFName, this->sceneInfo.vertexLayout, 1.0f, dev, queue);
                this->modelsMap[modelName] = std::move(model);
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

    // PREPARING_DESCRIPTOR_SETS {

    void setupDescriptorSetLayout(vks::VulkanDevice* dev)
    {
        uint32_t bindId = 0u;

    // SCENE_SPECIFIC {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
        std::cout << " >>> setupDescriptorSetLayout: adding bind of id: " << bindId << " - VertS UBO\n";
        setLayoutBindings.push_back(
            // Binding 0 : Vertex shader uniform buffer
            vks::initializers::descriptorSetLayoutBinding( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                           VK_SHADER_STAGE_VERTEX_BIT,
                                                           bindId++) );

        // Putting samplers for all types of textures used in this scene
        for (int i = 0; i < this->sceneInfo.getTextureSetSize(); i++)
        {
            std::cout << " >>> setupDescriptorSetLayout: adding bind of id: " << bindId << " - FragS samplers - one for every texture in TexSet\n";
            setLayoutBindings.push_back(
                // Binding: Fragment shader combined sampler
                vks::initializers::descriptorSetLayoutBinding( VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                               VK_SHADER_STAGE_FRAGMENT_BIT,
                                                               bindId++) );
        }
    // } // SCENE_SPECIFIC

        VkDescriptorSetLayoutCreateInfo descriptorLayout =
            vks::initializers::descriptorSetLayoutCreateInfo( setLayoutBindings.data(), setLayoutBindings.size());

        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(dev->logicalDevice, &descriptorLayout, nullptr, &this->descriptorSetLayout));
    }

    void setupDescriptorPool(vks::VulkanDevice* dev, VkDescriptorPool& descPool)
    { // This is fully scene specific.
        // One descriptor set per drawable object.
        const uint32_t descriptorCount = this->sceneInfo.getNeededDescriptorCount(); // Max number of sets.

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
                poolSizes.size(), // uint32_t poolSizeCount
                poolSizes.data(), // VkDescriptorPoolSize* pPoolSizes
                descriptorCount); // uint32_t maxSets

        VK_CHECK_RESULT(vkCreateDescriptorPool(dev->logicalDevice, &descriptorPoolInfo, nullptr, &descPool));
    }

    void setupDescriptorSet(vks::VulkanDevice* dev, VkDescriptorPool& descPool)
    { // This is fully scene specific.
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
                std::cout << "  >>> setupDescriptorSet: adding descriptor sets for entity: " << entityName << "\n";

                VkDescriptorSet descSet;
                VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->logicalDevice, &descripotrSetAllocInfo, &descSet));
                std::cout << "  >>> setupDescriptorSet: adding write descriptor set for UBO " << writeDescriptorSets.size() << "\n";
                writeDescriptorSets = {
                    // Binding 0 - unifirm buffer.
                    vks::initializers::writeDescriptorSet(descSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &this->uniformBuffers.scene.descriptor), // Binding 0 : Vertex shader uniform buffer
                };

                textures_set_name_t& texSetName = entity3dInfo.texturesSetName;
                TextureSetInfo& texSetInfo = this->sceneInfo.texturesSetInfoMap[texSetName];
                auto& texturesNames = texSetInfo.texturesNames;

                for (texture_name_t& texName : texturesNames)
                {
                    auto& textureDescriptor = this->texturesMap[texName].descriptor;
                    std::cout << "  >>> setupDescriptorSet: adding write descriptor set for sampler " << writeDescriptorSets.size() << ": " << texSetName << "/" << texName << "\n";
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

    // } // PREPARING_DESCRIPTOR_SETS

    // PREPARING_PIPELINES {

    void setupPipelineLayout(vks::VulkanDevice* dev)
    {
        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo( &this->descriptorSetLayout, 1); // 1 -> layout count.

        VK_CHECK_RESULT(vkCreatePipelineLayout(dev->logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &this->pipelineLayout));
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
        std::vector<VkVertexInputBindingDescription>   bindingDescriptions;
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
        // Only use the non-instanced input bindings and attribute descriptions
        inputState.vertexBindingDescriptionCount = 1; // Number of bingins (ubo etc.)
        inputState.vertexAttributeDescriptionCount = attributeDescriptions.size();

        pipelineCreateInfo.pVertexInputState = &inputState;

    // SCENE_SPECIFIC {

        { // All models pipeline creation
            for (auto& ent3dCreInf : this->sceneInfo.entities3dInfoMap)
            {
                entity_name_t entityName = ent3dCreInf.first;
                Entity3dInfo& entity3dInfo = ent3dCreInf.second;
                if (false == this->isPipelineAlreadyCreated(entityName))
                {
                    std::cout << " >>> preparePipelines: creating pipeline for entity: " << entityName << "\n";

                    shaders_set_name_t& shadSetName = entity3dInfo.shadersSetName;
                    ShaderSetInfo&      shadSetInfo = this->sceneInfo.shadersSetInfoMap[shadSetName];
                    auto& shaderNames = shadSetInfo.shadersNames;

                    uint8_t shadStCounter = 0u;
                    for (const shader_name_t& shadName : shaderNames) // Order is not relevant.
                    {
                        ShaderInfo& shaderInfo = sceneInfo.shadersInfoMap[shadName];

                        assert(shadName == shaderInfo.shaderName);

                        shader_filename_t shadFName = shaderInfo.shaderFilename;
                        shader_stage_t shadStage = shaderInfo.shaderStage;

                        std::cout << " >>> preparePipelines: loading shaders for entity: " << entityName << ", shader set: " << shadSetName << "\n";

                        shaderStages[shadStCounter++] = loadShader(dev->logicalDevice, assetsPath + "shaders/my_new_scene1/" + shadFName, shadStage, shaderModules);
                    }

                    VkPipeline pip;
                    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pip));
                    this->pipelinesMap[entityName] = std::move(pip);
                }
            }
        }

    // } // SCENE_SPECIFIC
    }

    // } // PREPARING_PIPELINES

    void buildCommandBuffer(VkCommandBuffer& drawCmdBuffer, uint32_t vertexBufferBindId, const VkDeviceSize* offsets)
    { // This is fully scene specific.
        for (auto& entCreInfMap : this->sceneInfo.entities3dInfoMap)
        {
            entity_name_t entName   = entCreInfMap.first;
            Entity3dInfo& entCreInf = entCreInfMap.second;

            model_name_t& modelName = entCreInf.modelName;

            auto& descrSet = this->descriptorSetsMap[entName];
            auto& pipeline = this->pipelinesMap[entName];
            auto& model    = this->modelsMap[modelName];

            std::cout << " >>> buildCommandBuffer: building draw command buffer for entity: " << entName << "\n";

            vkCmdBindDescriptorSets(drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineLayout, 0, 1, &descrSet, 0, NULL);
            vkCmdBindPipeline(drawCmdBuffer,       VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdBindVertexBuffers(drawCmdBuffer,  vertexBufferBindId, 1, &(model.vertices.buffer), offsets);
            vkCmdBindIndexBuffer(drawCmdBuffer,    model.indices.buffer,  0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(drawCmdBuffer,        model.indexCount,      1, 0, 0, 0);
        }
    }

// } // PREPARE

// RUNTIME {

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

    void copyDataToDeviceMemory()
    {
        memcpy(this->uniformBuffers.scene.mapped, &this->uboVS, sizeof(this->uboVS));
    }

// } // RUNTIME

// DESTROY {

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

// } // DESTROY
};


} // namespace vk229
