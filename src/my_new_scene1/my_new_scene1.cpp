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
      // {
      //    initxcbConnection();
      // }
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

        this->initVulkan();    // From base class.
        // {
        //     createInstance(settings.validation);
        //     vks::debug::setupDebugging(instance, debugReportFlags, VK_NULL_HANDLE);
        //     vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data())
        //     physicalDevice = physicalDevices[selectedDevice];
        //     vulkanDevice = new vks::VulkanDevice(physicalDevice);
        //     VkResult res = vulkanDevice->createLogicalDevice(enabledFeatures, enabledExtensions);
        //     vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.graphics, 0, &queue);
        //     VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);
        //     swapChain.connect(instance, physicalDevice, device);
        //     VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
        //     VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));
        //     VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.textOverlayComplete));
        //     submitInfo = vks::initializers::submitInfo();
        //     submitInfo.pWaitDstStageMask = &submitPipelineStages;
        //     submitInfo.waitSemaphoreCount = 1;
        //     submitInfo.pWaitSemaphores = &semaphores.presentComplete;
        //     submitInfo.signalSemaphoreCount = 1;
        //     submitInfo.pSignalSemaphores = &semaphores.renderComplete;
        // }

        this->setupWindow();   // From base class.

        this->initSwapchain(); // From base class.
        // {
        //     swapChain.initSurface(connection, window);
        // }

        this->prepare();
        // {
        //     createCommandPool();
        //     setupSwapChain();
        //     createCommandBuffers();
        //     setupDepthStencil();
        //     setupRenderPass();
        //     createPipelineCache();
        //     setupFrameBuffer();
        //
        //     loadAssets();
        //     prepareUniformBuffers();
        //     setupDescriptorSetLayout();
        //     setupDescriptorPool();
        //     setupDescriptorSet();
        //     preparePipelineLayout();
        //     preparePipelines();
        //     buildCommandBuffers(); // Overriden.
        //     prepared = true;
        // }

    }

    ~VulkanExample()
    {
        sceneData.destroy(device);
    }


// INIT {

    void initSceneCreateInfo()
    {
        // Scene definition here.
        // Structured like this for future reading from JSON-like file.

    // INPUT_DATA_RETRIEVED_FROM_FILE {

        std::vector<vk229::ModelInfo> modelsInfoVec = {
            {"box",    "box.obj"},
            {"light",  "light.obj"},
            {"floor",  "floor.obj"},
            {"cube1",  "cube1.obj"},
            {"cube2",  "cube2.obj"},
            {"cube3",  "cube3.obj"},
            {"monkey", "monkey.obj"},
            {"s1",     "s1.obj"},
            {"s2",     "s2.obj"},
            {"s3",     "s3.obj"},
            {"s4",     "s4.obj"},
            {"s5",     "s5.obj"},
            {"s6",     "s6.obj"},
            {"droid",  "full_droid_2.obj"},
            {"fluid",  "fluid.obj"},
        };

        std::vector<vk229::ShaderInfo> shadersInfoVec = {
            {"frag1", VK_SHADER_STAGE_VERTEX_BIT,   "default_transforms.vert.spv"},
            {"vert1", VK_SHADER_STAGE_FRAGMENT_BIT, "default_material.frag.spv"},
        };

        std::vector<vk229::TextureInfo> texturesInfoVec = {
            {"all_diffuse_C",       VK_FORMAT_BC3_UNORM_BLOCK, vk229::TexT::COLOR,        "all_diffuse_C.dds"},
            {"all_diffuse_DI",      VK_FORMAT_BC3_UNORM_BLOCK, vk229::TexT::DIFFUSE_DI,   "all_diffuse_DI.dds"},
            {"all_ao",              VK_FORMAT_BC3_UNORM_BLOCK, vk229::TexT::AO,           "all_ao.dds"},
            {"all_emit",            VK_FORMAT_BC3_UNORM_BLOCK, vk229::TexT::EMIT,         "all_emit.dds"},
            {"all_normal",          VK_FORMAT_B8G8R8A8_UNORM,  vk229::TexT::NORMAL,       "all_normal.dds"},
            {"reflection_center",   VK_FORMAT_B8G8R8A8_UNORM,  vk229::TexT::REFLECTION,   "reflection_center.dds"},
            {"reflection_droid",    VK_FORMAT_B8G8R8A8_UNORM,  vk229::TexT::REFLECTION,   "reflection_droid.dds"},
            {"reflection_monkey",   VK_FORMAT_B8G8R8A8_UNORM,  vk229::TexT::REFLECTION,   "reflection_monkey.dds"},
            {"reflection_s1",       VK_FORMAT_B8G8R8A8_UNORM,  vk229::TexT::REFLECTION,   "reflection_s1.dds"},
            {"reflection_s2",       VK_FORMAT_B8G8R8A8_UNORM,  vk229::TexT::REFLECTION,   "reflection_s2.dds"},
            {"reflection_s3",       VK_FORMAT_B8G8R8A8_UNORM,  vk229::TexT::REFLECTION,   "reflection_s3.dds"},
            {"reflection_s4",       VK_FORMAT_B8G8R8A8_UNORM,  vk229::TexT::REFLECTION,   "reflection_s4.dds"},
            {"reflection_s5",       VK_FORMAT_B8G8R8A8_UNORM,  vk229::TexT::REFLECTION,   "reflection_s5.dds"},
            {"reflection_s6",       VK_FORMAT_B8G8R8A8_UNORM,  vk229::TexT::REFLECTION,   "reflection_s6.dds"},
        };

        std::vector<vk229::MatrixInfo> matricesInfoVec = {
            {"mat1", glm::mat4x4()}, // TODO: make it used, fill correctly.
        };

        std::vector<vk229::TextureSetInfo> textureSetsInfoVec = {
            {
                "TEX_COMMON", {
                    "all_diffuse_C",
                    "all_diffuse_DI",
                    "all_ao",
                    "all_emit",
                    "all_normal",
                    "reflection_center",
                }
            },
            {
                "TEX_DROID", {
                    "all_diffuse_C",
                    "all_diffuse_DI",
                    "all_ao",
                    "all_emit",
                    "all_normal",
                    "reflection_droid",
                }
            },
            {
                "TEX_MONKEY", {
                    "all_diffuse_C",
                    "all_diffuse_DI",
                    "all_ao",
                    "all_emit",
                    "all_normal",
                    "reflection_monkey",
                }
            },
            {
                "TEX_S1", {
                    "all_diffuse_C",
                    "all_diffuse_DI",
                    "all_ao",
                    "all_emit",
                    "all_normal",
                    "reflection_s1",
                }
            },
            {
                "TEX_S2", {
                    "all_diffuse_C",
                    "all_diffuse_DI",
                    "all_ao",
                    "all_emit",
                    "all_normal",
                    "reflection_s2",
                }
            },
            {
                "TEX_S3", {
                    "all_diffuse_C",
                    "all_diffuse_DI",
                    "all_ao",
                    "all_emit",
                    "all_normal",
                    "reflection_s3",
                }
            },
            {
                "TEX_S4", {
                    "all_diffuse_C",
                    "all_diffuse_DI",
                    "all_ao",
                    "all_emit",
                    "all_normal",
                    "reflection_s4",
                }
            },
            {
                "TEX_S5", {
                    "all_diffuse_C",
                    "all_diffuse_DI",
                    "all_ao",
                    "all_emit",
                    "all_normal",
                    "reflection_s5",
                }
            },
            {
                "TEX_S6", {
                    "all_diffuse_C",
                    "all_diffuse_DI",
                    "all_ao",
                    "all_emit",
                    "all_normal",
                    "reflection_s6",
                }
            },
        };

        std::vector<vk229::ShaderSetInfo> shadersSetsInfoVec = {
            {
                "SHADER_SET0", {
                    "frag1",
                    "vert1",
                }
            },
            {
                "SHADER_SET1", {
                    "frag1",
                    "vert1",
                }
            },
        };

        std::vector<vk229::Entity3dInfo> entitiesInfoVec = {
            {"Box",     "box",      "mat1", "TEX_COMMON",   "SHADER_SET0"},
            {"Light",   "light",    "mat1", "TEX_COMMON",   "SHADER_SET0"},
            {"Floor",   "floor",    "mat1", "TEX_COMMON",   "SHADER_SET0"},
            {"Cube1",   "cube1",    "mat1", "TEX_MONKEY",   "SHADER_SET0"},
            {"Cube2",   "cube2",    "mat1", "TEX_MONKEY",   "SHADER_SET0"},
            {"Cube3",   "cube3",    "mat1", "TEX_MONKEY",   "SHADER_SET0"},
            {"Monkey",  "monkey",   "mat1", "TEX_MONKEY",   "SHADER_SET0"},
            {"S1",      "s1",       "mat1", "TEX_S1",       "SHADER_SET0"},
            {"S2",      "s2",       "mat1", "TEX_S2",       "SHADER_SET0"},
            {"S3",      "s3",       "mat1", "TEX_S3",       "SHADER_SET0"},
            {"S4",      "s4",       "mat1", "TEX_S4",       "SHADER_SET0"},
            {"S5",      "s5",       "mat1", "TEX_S5",       "SHADER_SET0"},
            {"S6",      "s6",       "mat1", "TEX_S6",       "SHADER_SET0"},
            {"Droid",   "droid",    "mat1", "TEX_DROID",    "SHADER_SET0"},
            {"Fluid",   "fluid",    "mat1", "TEX_COMMON",   "SHADER_SET0"},
        };

    // } // INPUT_DATA_RETRIEVED_FROM_FILE

    // PUTTING_DATA_INTO_MAPS {

        sceneData.sceneInfo.fillModelsInfoMap(modelsInfoVec);
        sceneData.sceneInfo.fillShadersInfoMap(shadersInfoVec);
        sceneData.sceneInfo.fillTexturesInfoMap(texturesInfoVec);
        sceneData.sceneInfo.fillMatricesInfoMap(matricesInfoVec);
        sceneData.sceneInfo.fillTexturesSetInfoMap(textureSetsInfoVec);
        sceneData.sceneInfo.fillShadersSetInfoMap(shadersSetsInfoVec);
        sceneData.sceneInfo.fillEntities3dInfoMap(entitiesInfoVec);

    // } // PUTTING_DATA_INTO_MAPS
    }

    // void VulkanExampleBase::initVulkan();

    // xcb_window_t VulkanExampleBase::setupWindow();

    // void VulkanExampleBase::initSwapchain();

    void prepare() override
    {
        VulkanExampleBase::prepare();
        // {
        //     createCommandPool();
        //     setupSwapChain();
        //     createCommandBuffers();
        //     setupDepthStencil();
        //     setupRenderPass();
        //     createPipelineCache();
        //     setupFrameBuffer();
        //     // Setup text overlay (shaders + whole pipeline).
        // }

        loadAssets();
        prepareUniformBuffers();
        setupDescriptorSetLayout();
        setupDescriptorPool();
        setupDescriptorSet();
        preparePipelineLayout();
        preparePipelines();
        buildCommandBuffers(); // Overriden.
        prepared = true;
    }

// } // INIT


// PREPARE {

    void loadAssets()
    {
        sceneData.loadTextures(vulkanDevice, queue, getAssetPath());
        sceneData.loadModels(vulkanDevice, queue, getAssetPath());
    }

    void prepareUniformBuffers()
    {
        sceneData.prepareUniformBuffers(vulkanDevice, camera.matrices.view, camera.matrices.perspective);
    }

    void setupDescriptorSetLayout()
    {
        sceneData.setupDescriptorSetLayout(vulkanDevice);
    }

    void setupDescriptorPool()
    {
        sceneData.setupDescriptorPool(vulkanDevice, descriptorPool);
    }

    void setupDescriptorSet()
    {
        sceneData.setupDescriptorSet(vulkanDevice, descriptorPool);
    }

    void preparePipelineLayout()
    {
        sceneData.setupPipelineLayout(vulkanDevice);
    }

    void preparePipelines()
    {
        sceneData.preparePipelines(vulkanDevice, renderPass, pipelineCache, VERTEX_BUFFER_BIND_ID, getAssetPath(), shaderModules);
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

            // Scene part.
            sceneData.buildCommandBuffer(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, offsets);

            vkCmdEndRenderPass(drawCmdBuffers[i]);
            VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
        }
    }

// } // PREPARE

// RUNTIME {

    // void VulkanExampleBase::renderLoop() {
    //    xcb_flush(connection);
    //    while (!quit)
    //    {
    //        auto tStart = now();
    //        if (viewUpdated) { viewUpdated = false; viewChanged(); }
    //        while (event = xcb_poll_for_event(connection)) { handleEvent(event); free(event); }
    //        render();
    //        frameCounter++;
    //        auto tEnd = now(); auto tDiff = tEnd-tStart;
    //        frameTimer = tDiff / 1000.0f;
    //        camera.update(frameTimer);
    //        if (camera.moving()) { viewUpdated = true; }
    //        if (!paused) { timer += timerSpeed * frameTimer; if (timer > 1.0) { timer -= 1.0f; } }
    //        fpsTimer += (float)tDiff;
    //        if (fpsTimer > 1000.0f) {// code;}
    //    }
    //    vkDeviceWaitIdle(device);
    //}


    virtual void viewChanged() override
    {
        updateUniformBuffer(true);
    }

    // void VulkanExampleBase::handleEvent(const xcb_generic_event_t *event);

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

    void updateUniformBuffer(bool viewChanged)
    {
        sceneData.updateUniformBuffers(viewChanged, camera.matrices.view, camera.matrices.perspective);
    }

    // Camera::update(frameTimer);

    // Camera::moving()

    virtual void getOverlayText(VulkanTextOverlay *textOverlay) override
    {
        textOverlay->addText("LMB to rotate, WSAD to move", 5.0f, 105.0f, VulkanTextOverlay::alignLeft);
    }

// } // RUNTIME
};

// MAIN {

std::unique_ptr<VulkanExample> vulkanExample;
static void handleEvent(const xcb_generic_event_t *event)
{
    if (vulkanExample.get() != NULL)
    {
        vulkanExample->handleEvent(event);
    }
}

int main(const int argc, const char *argv[])
{
    for (size_t i = 0; i < argc; i++) { VulkanExample::args.push_back(argv[i]); };

    vulkanExample.reset(new VulkanExample());

    vulkanExample->renderLoop();

    //delete(vulkanExample);

    return 0;
}

// } // MAIN
