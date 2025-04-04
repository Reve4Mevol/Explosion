//
// Created by Zach Lee on 2022/4/4.
//

#include <RHI/Vulkan/SwapChain.h>
#include <RHI/Vulkan/Instance.h>
#include <RHI/Vulkan/Common.h>
#include <RHI/Vulkan/Device.h>
#include <RHI/Vulkan/Gpu.h>
#include <RHI/Vulkan/Texture.h>
#include <RHI/Vulkan/Queue.h>
#include <RHI/Vulkan/Surface.h>
#include <RHI/Vulkan/Synchronous.h>

namespace RHI::Vulkan {
    VulkanSwapChain::VulkanSwapChain(VulkanDevice& inDevice, const SwapChainCreateInfo& inCreateInfo)
        : SwapChain(inCreateInfo)
        , device(inDevice)
        , nativeSwapChain(VK_NULL_HANDLE)
        , nativeQueue(VK_NULL_HANDLE)
    {
        CreateNativeSwapChain(inCreateInfo);
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        const auto vkDevice = device.GetNative();
        vkDeviceWaitIdle(vkDevice);

        for (const auto& tex : textures) {
            delete tex;
        }
        textures.clear();

        if (nativeSwapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(vkDevice, nativeSwapChain, nullptr);
        }
    }

    uint8_t VulkanSwapChain::GetTextureNum()
    {
        return swapChainImageCount;
    }

    Texture* VulkanSwapChain::GetTexture(const uint8_t inIndex)
    {
        return textures[inIndex];
    }

    uint8_t VulkanSwapChain::AcquireBackTexture(Semaphore* inSignalSemaphore)
    {
        const auto& vulkanSignalSemaphore = static_cast<VulkanSemaphore&>(*inSignalSemaphore);
        const auto result = vkAcquireNextImageKHR(device.GetNative(), nativeSwapChain, UINT64_MAX, vulkanSignalSemaphore.GetNative(), VK_NULL_HANDLE, &currentImage);
        Assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
        return currentImage;
    }

    void VulkanSwapChain::Present(RHI::Semaphore* inWaitSemaphore)
    {
        const auto& vulkanWaitSemaphore = static_cast<VulkanSemaphore&>(*inWaitSemaphore);
        const std::vector waitSemaphores { vulkanWaitSemaphore.GetNative() };

        VkPresentInfoKHR presetInfo = {};
        presetInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presetInfo.swapchainCount = 1;
        presetInfo.pSwapchains = &nativeSwapChain;
        presetInfo.waitSemaphoreCount = waitSemaphores.size();
        presetInfo.pWaitSemaphores = waitSemaphores.data();
        presetInfo.pImageIndices = &currentImage;

        const auto result = vkQueuePresentKHR(nativeQueue, &presetInfo);
        Assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
    }

    void VulkanSwapChain::CreateNativeSwapChain(const SwapChainCreateInfo& inCreateInfo)
    {
        const auto vkDevice = device.GetNative();
        const auto* mQueue = static_cast<VulkanQueue*>(inCreateInfo.presentQueue);
        Assert(mQueue);
        auto* vkSurface = static_cast<VulkanSurface*>(inCreateInfo.surface);
        Assert(vkSurface);
        nativeQueue = mQueue->GetNative();
        const auto surface = vkSurface->GetNative();

        VkSurfaceCapabilitiesKHR surfaceCap;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetGpu().GetNative(), surface, &surfaceCap);

        VkExtent2D extent = {static_cast<uint32_t>(inCreateInfo.width), static_cast<uint32_t>(inCreateInfo.height)};
        extent.width = std::clamp(extent.width, surfaceCap.minImageExtent.width, surfaceCap.maxImageExtent.width);
        extent.height = std::clamp(extent.height, surfaceCap.minImageExtent.height, surfaceCap.maxImageExtent.height);

        Assert(device.CheckSwapChainFormatSupport(vkSurface, inCreateInfo.format));
        auto supportedFormat = EnumCast<PixelFormat, VkFormat>(inCreateInfo.format);
        auto colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

        uint32_t presentModeCount = 0;
        std::vector<VkPresentModeKHR> presentModes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device.GetGpu().GetNative(), surface, &presentModeCount, nullptr);
        Assert(presentModeCount != 0);
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device.GetGpu().GetNative(), surface, &presentModeCount, presentModes.data());

        VkPresentModeKHR supportedMode = EnumCast<PresentMode, VkPresentModeKHR>(inCreateInfo.presentMode);
        {
            Assert(!presentModes.empty());
            const auto iter = std::ranges::find_if(
                presentModes,
                [supportedMode](const VkPresentModeKHR mode) { return mode == supportedMode; });
            Assert(iter != presentModes.end());
        }

        swapChainImageCount = std::clamp(static_cast<uint32_t>(inCreateInfo.textureNum), surfaceCap.minImageCount, surfaceCap.maxImageCount);
        VkSwapchainCreateInfoKHR swapChainInfo = {};
        swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainInfo.surface = surface;
        swapChainInfo.minImageCount = swapChainImageCount;
        swapChainInfo.imageFormat = supportedFormat;
        swapChainInfo.imageColorSpace = colorSpace;
        swapChainInfo.presentMode = supportedMode;
        swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapChainInfo.imageExtent = extent;
        swapChainInfo.clipped = VK_TRUE;
        swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainInfo.imageArrayLayers = 1;
        swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        Assert(vkCreateSwapchainKHR(vkDevice, &swapChainInfo, nullptr, &nativeSwapChain) == VK_SUCCESS);

        TextureCreateInfo textureInfo = {};
        textureInfo.format = inCreateInfo.format;
        textureInfo.usages = TextureUsageBits::renderAttachment;
        textureInfo.mipLevels = 1;
        textureInfo.samples = 1;
        textureInfo.dimension = TextureDimension::t2D;
        textureInfo.width = extent.width;
        textureInfo.height = extent.height;
        textureInfo.depthOrArraySize = 1;
        textureInfo.initialState = TextureState::present;

        vkGetSwapchainImagesKHR(device.GetNative(), nativeSwapChain, &swapChainImageCount, nullptr);
        std::vector<VkImage> swapChainImages(swapChainImageCount);
        vkGetSwapchainImagesKHR(device.GetNative(), nativeSwapChain, &swapChainImageCount, swapChainImages.data());
        for (const auto& image : swapChainImages) {
            textures.emplace_back(new VulkanTexture(device, textureInfo, image));
        }
        swapChainImageCount = static_cast<uint32_t>(swapChainImages.size());
    }
}