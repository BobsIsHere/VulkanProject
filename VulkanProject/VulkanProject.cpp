#include "VulkanProject.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

void VulkanProject::Run()
{
    // Make Window
    m_pCamera = std::make_unique<Camera>(glm::vec3(3.f, 0.5f, 0.f));
    m_pWindow = std::make_unique<Window>(m_pCamera.get());
    m_pWindow->Initialize(utils::WINDOW_WIDTH, utils::WINDOW_HEIGHT, "Vulkan Window");

    // Initialize Vulkan
    m_pVulkanInstance = std::make_unique<VulkanInstance>();
    m_pVulkanDevice = std::make_unique<VulkanDevice>(m_pVulkanInstance.get(), m_pWindow.get());
    m_pVulkanSwapChain = std::make_unique<VulkanSwapChain>(m_pWindow.get(), m_pVulkanDevice.get());
    m_pVulkanRenderPass = std::make_unique<VulkanRenderPass>(m_pVulkanDevice.get(), m_pVulkanSwapChain.get());
    m_pVulkanCommandPool = std::make_unique<VulkanCommandPool>(m_pVulkanDevice.get());
    m_pVulkanDescriptorPool = std::make_unique<VulkanDescriptorPool>(m_pVulkanDevice.get());

    // Initialize Graphics Pipeline
    m_pGraphicsPipeline = std::make_unique<GraphicsPipeline>(m_pVulkanDevice.get(), m_pVulkanRenderPass.get());

    m_pVulkanCommandBuffers.resize(utils::MAX_FRAMES_IN_FLIGHT);
    m_pVulkanDescriptorSets.resize(utils::MAX_FRAMES_IN_FLIGHT);
    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        m_pVulkanCommandBuffers[idx] = std::make_unique<VulkanCommandBuffer>();
        m_pVulkanDescriptorSets[idx] = std::make_unique<VulkanDescriptorSet>(m_pVulkanDevice.get(), m_pVulkanDescriptorPool.get());
    }

    // Initialize Buffers
    m_pUniformBuffers.resize(utils::MAX_FRAMES_IN_FLIGHT);

    m_pVertexBuffer = std::make_unique<VertexBuffer>(m_pVulkanDevice.get(), m_pVulkanCommandPool.get());
    m_pIndexBuffer = std::make_unique<IndexBuffer>(m_pVulkanDevice.get(), m_pVulkanCommandPool.get());

    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        m_pUniformBuffers[idx] = std::make_unique<UniformBuffer>(m_pVulkanDevice.get(), m_pVulkanCommandPool.get());
    }

    // Initialize Rendering
    m_pRenderer = std::make_unique<Renderer>(m_pVulkanDevice.get(), m_pVulkanSwapChain.get(), m_pVulkanRenderPass.get(), m_pWindow.get(), m_pCamera.get());

    m_pVikingModel = std::make_unique<Model>("models/viking_room.obj");
    m_pVikingTexture = std::make_unique<Texture>(m_pVulkanDevice.get(), m_pVulkanCommandPool.get(), "textures/viking_room.png");

    InitVulkan();
    InitImGui();
    MainLoop();
    CleanupVulkan();
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
    m_pRenderer->CreateFramebuffers();
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
        m_pVulkanCommandBuffers[idx]->Create(m_pVulkanDevice.get(), m_pVulkanCommandPool.get());
        m_pVulkanDescriptorSets[idx]->Create(m_pGraphicsPipeline.get(), m_pUniformBuffers[idx].get(),
            m_pVikingTexture->GetImageView(), m_pVikingTexture->GetSampler());
    }

    m_pRenderer->CreateSyncObjects();
}

void VulkanProject::InitImGui()
{
	std::unique_ptr<VulkanCommandPool> commandPool{ std::make_unique<VulkanCommandPool>(m_pVulkanDevice.get()) };

    //1. Create descriptor pool for IMGUI
    VkDescriptorPoolSize imguiPoolSize[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

	VkDescriptorPoolCreateInfo imguiPoolInfo{};
    imguiPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	imguiPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	imguiPoolInfo.maxSets = 1000;
    imguiPoolInfo.poolSizeCount = std::size(imguiPoolSize);
    imguiPoolInfo.pPoolSizes = imguiPoolSize;

    m_ImGuiPool = VK_NULL_HANDLE;
    if (vkCreateDescriptorPool(m_pVulkanDevice->GetDevice(), &imguiPoolInfo, nullptr, &m_ImGuiPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create ImGui DescriptorPool");
    }

    // 2. Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

    // 3. Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(m_pWindow->GetWindow(), true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_pVulkanInstance->GetInstance();
    init_info.PhysicalDevice = m_pVulkanDevice->GetPhysicalDevice();
    init_info.Device = m_pVulkanDevice->GetDevice();
    init_info.QueueFamily = m_pVulkanDevice->FindQueueFamilies(m_pVulkanDevice->GetPhysicalDevice()).graphicsFamily.value();
    init_info.Queue = m_pVulkanDevice->GetGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_ImGuiPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = utils::MAX_FRAMES_IN_FLIGHT;
    init_info.ImageCount = utils::MAX_FRAMES_IN_FLIGHT;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.RenderPass = m_pVulkanRenderPass->GetRenderPass();

    ImGui_ImplVulkan_Init(&init_info);
}

void VulkanProject::MainLoopImGui()
{
    // Start ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug");

    // FPS & Frame Time
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

    static float frameTimes[100] = {};
    static int frameIndex = 0;
    frameTimes[frameIndex] = 1000.0f / io.Framerate;
    frameIndex = (frameIndex + 1) % 100;
    ImGui::PlotLines("Frame Times (ms)", frameTimes, IM_ARRAYSIZE(frameTimes), frameIndex, nullptr, 0.0f, 50.0f, ImVec2(0, 80));

    ImGui::Separator();

    // Vulkan info
    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(m_pVulkanDevice->GetPhysicalDevice(), &deviceProperties);

    ImGui::Text("Vulkan Device: %s", deviceProperties.deviceName);
    ImGui::Text("API Version: %d.%d.%d",
        VK_VERSION_MAJOR(deviceProperties.apiVersion),
        VK_VERSION_MINOR(deviceProperties.apiVersion),
        VK_VERSION_PATCH(deviceProperties.apiVersion));
    ImGui::Text("Driver Version: %u", deviceProperties.driverVersion);

    // VRAM info (if available)
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(m_pVulkanDevice->GetPhysicalDevice(), &memProperties);

    // A total VRAM calculation (device local heaps)
    VkDeviceSize totalVRAM = 0;
    for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++)
    {
        if (memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            totalVRAM += memProperties.memoryHeaps[i].size;
    }
    ImGui::Text("Total VRAM: %.2f MB", totalVRAM / (1024.0f * 1024.0f));

    ImGui::End();

    ImGui::Render();

    m_pRenderer->DrawFrame(
        m_pUniformBuffers,
        m_pVertexBuffer.get(), m_pIndexBuffer.get(),
        m_pVulkanCommandBuffers,
        m_pVulkanCommandPool.get(),
        m_pGraphicsPipeline.get(),
        m_pVulkanDescriptorSets,
        m_pVikingModel->GetIndices(),
        ImGui::GetDrawData());
}

void VulkanProject::MainLoop()
{
    float lastFrameTime{ static_cast<float>(glfwGetTime()) };

    while (!glfwWindowShouldClose(m_pWindow->GetWindow()))
    {
        const float currentFrameTime{ static_cast<float>(glfwGetTime()) };
        const float deltaTime{ currentFrameTime - lastFrameTime };
        lastFrameTime = currentFrameTime;

        glfwPollEvents();
		MainLoopImGui();
    }

    vkDeviceWaitIdle(m_pVulkanDevice->GetDevice());
}

void VulkanProject::CleanupVulkan()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	vkDestroyDescriptorPool(m_pVulkanDevice->GetDevice(), m_ImGuiPool, nullptr);

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