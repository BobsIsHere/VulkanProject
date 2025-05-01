#define STB_IMAGE_IMPLEMENTATION
#include <3rdParty/stb_image.h>

#include "VulkanProject.h"

void VulkanProject::Run()
{
    // Make Window
    m_pWindow = new Window();
    m_pWindow->Initialize(utils::WINDOW_WIDTH, utils::WINDOW_HEIGHT, "Vulkan Window");

    // Initialize Vulkan
    m_pVulkanInstance = new VulkanInstance();
    m_pVulkanDevice = new VulkanDevice(m_pVulkanInstance, m_pWindow);
    m_pVulkanSwapChain = new VulkanSwapChain(m_pWindow, m_pVulkanDevice);
    m_pVulkanRenderPass = new VulkanRenderPass(m_pVulkanDevice, m_pVulkanSwapChain);
    m_pVulkanCommandPool = new VulkanCommandPool(m_pVulkanDevice);
    m_pVulkanDescriptorPool = new VulkanDescriptorPool(m_pVulkanDevice);

    // Initialize Graphics Pipeline
    m_pGraphicsPipeline = new GraphicsPipeline(m_pVulkanDevice, m_pVulkanRenderPass);

    m_pVulkanCommandBuffers.resize(utils::MAX_FRAMES_IN_FLIGHT);
    m_pVulkanDescriptorSets.resize(utils::MAX_FRAMES_IN_FLIGHT);
    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        m_pVulkanCommandBuffers[idx] = new VulkanCommandBuffer();
        m_pVulkanDescriptorSets[idx] = new VulkanDescriptorSet(m_pVulkanDevice, m_pVulkanDescriptorPool);
    }

    // Initialize Buffers
    m_pUniformBuffers.resize(utils::MAX_FRAMES_IN_FLIGHT);

    m_pVertexBuffer = new VertexBuffer(m_pVulkanDevice, m_pVulkanCommandPool);
    m_pIndexBuffer = new IndexBuffer(m_pVulkanDevice, m_pVulkanCommandPool);

    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        m_pUniformBuffers[idx] = new UniformBuffer(m_pVulkanDevice, m_pVulkanCommandPool);
    }

    m_pRenderer = new Renderer(m_pVulkanDevice, m_pVulkanSwapChain, m_pVulkanRenderPass, m_pWindow);

    m_pVikingModel = new Model("models/viking_room.obj");
    m_pVikingTexture = new Texture(m_pVulkanDevice, m_pVulkanCommandPool, "textures/viking_room.png");

    InitVulkan();
    MainLoop();
    CleanupVulkan();
    CleanupResources();
}

void VulkanProject::InitVulkan()
{
    m_pVulkanInstance->Initialize("Hello World!");
    m_pVulkanInstance->SetupDebugMessenger();

    m_pVulkanDevice->CreateSurface();
    m_pVulkanDevice->PickPhysicalDevice();
    m_pVulkanDevice->CreateLogicalDevice();

    m_pVulkanSwapChain->Create();

    m_pVulkanSwapChain->CreateImageViews();
    m_pVulkanRenderPass->Create();
    m_pGraphicsPipeline->CreateDescriptorSetLayout();
    m_pGraphicsPipeline->CreatePipeline();
    m_pVulkanCommandPool->Create();
    m_pRenderer->CreateDepthResources();
    m_pRenderer->CreateFrameBuffers();
    m_pVikingTexture->CreateTextureImage();
    m_pVikingTexture->CreateTextureImageView();
    m_pVikingTexture->CreateTextureSampler();
    m_pVikingModel->LoadModel();

    // Create Buffers
    m_pVertexBuffer->CreateVertexBuffer(m_pVikingModel->GetVertices());
    m_pIndexBuffer->CreateIndexBuffer(m_pVikingModel->GetIndices());
    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        m_pUniformBuffers[idx]->CreateUniformBuffer();
    }

    m_pVulkanDescriptorPool->Create();
    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        m_pVulkanCommandBuffers[idx]->Create(m_pVulkanDevice, m_pVulkanCommandPool);
        m_pVulkanDescriptorSets[idx]->Create(m_pGraphicsPipeline, m_pUniformBuffers[idx], 
            m_pVikingTexture->GetImageView(), m_pVikingTexture->GetSampler());
    }

    m_pRenderer->CreateSyncObjects();
}

void VulkanProject::MainLoop()
{
    while (!glfwWindowShouldClose(m_pWindow->GetWindow()))
    {
        glfwPollEvents();
        m_pRenderer->DrawFrame(m_pUniformBuffers, m_pVertexBuffer, m_pIndexBuffer, m_pVulkanCommandBuffers, m_pGraphicsPipeline, m_pVulkanDescriptorSets,
            m_pVikingModel->GetIndices());
    }

    vkDeviceWaitIdle(m_pVulkanDevice->GetDevice());
}

void VulkanProject::CleanupVulkan()
{
    const auto& device{ m_pVulkanDevice->GetDevice() };

    m_pRenderer->CleanupSwapChain();

    m_pGraphicsPipeline->CleanupPipeline();

    m_pVulkanRenderPass->Cleanup();

    for (size_t i = 0; i < utils::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        m_pUniformBuffers[i]->Cleanup();
    }

    m_pVulkanDescriptorPool->Cleanup();

    m_pVikingTexture->CleanupSampler();
    m_pVikingTexture->Cleanup();

    m_pGraphicsPipeline->CleanupDescriptorSetLayout();

    m_pIndexBuffer->Cleanup();
    m_pVertexBuffer->Cleanup();

    m_pRenderer->Cleanup();

    m_pVulkanCommandPool->Cleanup();

    m_pVulkanDevice->Cleanup();

    m_pVulkanInstance->Cleanup();

    glfwDestroyWindow(m_pWindow->GetWindow());

    glfwTerminate();
}

void VulkanProject::CleanupResources()
{
    delete m_pVikingModel;
    m_pVikingModel = nullptr;

    delete m_pRenderer;
    m_pRenderer = nullptr;

    delete m_pVulkanRenderPass;
    m_pVulkanRenderPass = nullptr;

    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        delete m_pUniformBuffers[idx];
        m_pUniformBuffers[idx] = nullptr;

        delete m_pVulkanCommandBuffers[idx];
        m_pVulkanCommandBuffers[idx] = nullptr;
    }

    delete m_pVulkanDescriptorPool;
    m_pVulkanDescriptorPool = nullptr;

    delete m_pGraphicsPipeline;
    m_pGraphicsPipeline = nullptr;

    delete m_pIndexBuffer;
    m_pIndexBuffer = nullptr;

    delete m_pVertexBuffer;
    m_pVertexBuffer = nullptr;

    delete m_pVulkanCommandPool;
    m_pVulkanCommandPool = nullptr;

    delete m_pVulkanDevice;
    m_pVulkanDevice = nullptr;

    delete m_pVulkanInstance;
    m_pVulkanInstance = nullptr;
}