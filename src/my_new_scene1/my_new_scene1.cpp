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
#include <HelperStructsAndFuncs.hpp>

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

class VulkanExample : public VulkanExampleBase
{
public:
    vk229::SceneData sceneData;

    VulkanExample() :
        VulkanExampleBase(ENABLE_VALIDATION)
    {
        title = "Vulkan Example - a scene";
        enableTextOverlay = true;
        srand(time(NULL));
        cameraPos = { 3.0f, 6.0f, -5.0f };
        rotation = {-45.0f, 25.0f, 0.0f };
        zoom = -10.0f;
        camera.setPerspective(80.0f, (float)width / (float)height, 1.0f/128.0f, 128.0f);
        camera.type = Camera::firstperson;
        camera.setRotation(rotation);
        camera.setPosition(cameraPos);
        camera.rotationSpeed = 0.25f;
        camera.movementSpeed = 2.0f;

        // INIT
        this->initSceneCreateInfo();
        this->initVulkan();
        this->setupWindow();
        this->initSwapchain();
        this->prepare();
    }

    ~VulkanExample()
    {
        sceneData.destroy(device);
    }

    void initSceneCreateInfo()
    {
        // Scene definition here.
        // Structured like this for future reading from JSON-like file.

        sceneData.sceneInfo.texturesSetInfoMap = {
            { "TEX_SET0",
              {{
                  {"0COLOR",       {"all_diffuse_C.dds",     VK_FORMAT_BC3_UNORM_BLOCK}},
                  {"1DIFFUSE_DI",  {"all_diffuse_DI.dds",    VK_FORMAT_BC3_UNORM_BLOCK}},
                  {"2AO",          {"all_ao.dds",            VK_FORMAT_BC3_UNORM_BLOCK}},
                  {"3EMIT",        {"all_emit.dds",          VK_FORMAT_BC3_UNORM_BLOCK}},
                  {"4NORMAL",      {"all_normal.dds",        VK_FORMAT_B8G8R8A8_UNORM }}, // BC5 is good for normal, but uncompressed is better.
                  {"5REFLECTION",  {"reflection_center.dds", VK_FORMAT_B8G8R8A8_UNORM }}
              }}
            },
            { "TEX_SET1",
              {{
                  {"0COLOR",       {"all_diffuse_C.dds",     VK_FORMAT_BC3_UNORM_BLOCK}},
                  {"1DIFFUSE_DI",  {"all_diffuse_DI.dds",    VK_FORMAT_BC3_UNORM_BLOCK}},
                  {"2AO",          {"all_ao.dds",            VK_FORMAT_BC3_UNORM_BLOCK}},
                  {"3EMIT",        {"all_emit.dds",          VK_FORMAT_BC3_UNORM_BLOCK}},
                  {"4NORMAL",      {"all_normal.dds",        VK_FORMAT_B8G8R8A8_UNORM }}, // BC5 is good for normal, but uncompressed is better.
                  {"5REFLECTION",  {"reflection_center.dds", VK_FORMAT_B8G8R8A8_UNORM }}
              }}
            },
        };

        sceneData.sceneInfo.shadersSetInfoMap = {
            { "SHADER_SET0",
                {{
                    {"VERT", {"default_transforms.vert.spv", VK_SHADER_STAGE_VERTEX_BIT}},
                    {"FRAG", {"default_material.frag.spv",   VK_SHADER_STAGE_FRAGMENT_BIT}}
                }},
            },
            { "SHADER_SET1",
                {{
                    {"VERT", {"default_transforms.vert.spv", VK_SHADER_STAGE_VERTEX_BIT}},
                    {"FRAG", {"default_material.frag.spv",   VK_SHADER_STAGE_FRAGMENT_BIT}}
                }},
            },
        };

        sceneData.sceneInfo.entities3dInfoMap = {
            {"box",    {{"box.obj"},   "TEX_SET1", "SHADER_SET1"}},
            {"light",  {{"light.obj"}, "TEX_SET1", "SHADER_SET1"}},
            {"floor",  {{"floor.obj"}, "TEX_SET1", "SHADER_SET1"}},
            {"cube1",  {{"cube1.obj"}, "TEX_SET1", "SHADER_SET1"}},
            {"cube2",  {{"cube2.obj"}, "TEX_SET1", "SHADER_SET1"}},
            {"cube3",  {{"cube3.obj"}, "TEX_SET1", "SHADER_SET1"}},
            {"monkey", {{"monkey.obj"},"TEX_SET1", "SHADER_SET1"}},
            {"s1",     {{"s1.obj"},    "TEX_SET1", "SHADER_SET1"}},
            {"s2",     {{"s2.obj"},    "TEX_SET1", "SHADER_SET1"}},
            {"s3",     {{"s3.obj"},    "TEX_SET1", "SHADER_SET1"}},
            {"s4",     {{"s4.obj"},    "TEX_SET1", "SHADER_SET1"}},
            {"s5",     {{"s5.obj"},    "TEX_SET1", "SHADER_SET1"}},
            {"s6",     {{"s6.obj"},    "TEX_SET1", "SHADER_SET1"}},
            {"full_droid", {{"full_droid.obj"}, "TEX_SET1", "SHADER_SET1"}},
        };
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

            sceneData.buildCommandBuffer(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, offsets);

            vkCmdEndRenderPass(drawCmdBuffers[i]);
            VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
        }
    }

    void loadAssets()
    {
        sceneData.loadTextures(vulkanDevice, queue, getAssetPath());
        sceneData.loadModels(vulkanDevice, queue, getAssetPath());
    }

    void setupDescriptorPool()
    {
        sceneData.setupDescriptorPool(vulkanDevice, descriptorPool);
    }

    void setupDescriptorSetLayout()
    {
        sceneData.setupDescriptorSetLayout(vulkanDevice);
    }

    void setupDescriptorSet()
    {
        sceneData.setupDescriptorSet(vulkanDevice, descriptorPool);
    }

    void preparePipelines()
    {
        sceneData.preparePipelines(vulkanDevice, renderPass, pipelineCache, VERTEX_BUFFER_BIND_ID, getAssetPath(), shaderModules);
    }

    void prepareUniformBuffers()
    {
        sceneData.prepareUniformBuffers(vulkanDevice, camera.matrices.view, camera.matrices.perspective);
    }

    void updateUniformBuffer(bool viewChanged)
    {
        sceneData.updateUniformBuffers(viewChanged, camera.matrices.view, camera.matrices.perspective);
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
        textOverlay->addText("LMB to rotate, WSAD to move", 5.0f, 105.0f, VulkanTextOverlay::alignLeft);
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
