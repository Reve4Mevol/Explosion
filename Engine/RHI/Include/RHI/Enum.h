//
// Created by johnk on 1/1/2022.
//

#ifndef EXPLOSION_RHI_ENUM_H
#define EXPLOSION_RHI_ENUM_H

#include <string>

namespace RHI {
    const std::string RHI_INSTANCE_EXT_NAME_SURFACE = "EXT_INS_Surface";
    const std::string RHI_INSTANCE_EXT_NAME_WINDOWS_SURFACE = "EXT_INS_WindowsSurface";

    const std::string RHI_DEVICE_EXT_NAME_SWAP_CHAIN = "EXT_DVC_SwapChain";
}

namespace RHI {
    enum class QueueFamilyType {
        GRAPHICS = 0,
        SECONDARY,
        COMPUTE,
        COPY,
        MAX
    };

    enum class PixelFormat {
        R8G8B8A8_UNORM = 0,
        MAX
    };

    enum class DeviceMemoryType {
        HOST_VISIBLE = 0,
        DEVICE_LOCAL,
        MAX
    };

    enum class ImageType {
        IMAGE_1D = 0,
        IMAGE_2D,
        IMAGE_3D,
        MAX
    };

    enum class ImageViewType {
        IMAGE_VIEW_1D = 0,
        IMAGE_VIEW_2D,
        IMAGE_VIEW_3D,
        IMAGE_VIEW_CUBE,
        IMAGE_VIEW_1D_ARRAY,
        IMAGE_VIEW_2D_ARRAY,
        IMAGE_VIEW_CUBE_ARRAY,
        MAX
    };

    enum class ComponentSwizzle {
        IDENTITY = 0,
        ZERO,
        ONE,
        R,
        G,
        B,
        A,
        MAX
    };

    enum class ShaderStage {
        VERTEX = 0,
        FRAGMENT,
        MAX
    };

    enum class AttachmentLoadOp {
        WHATEVER = 0,
        CLEAR,
        MAX
    };

    enum class AttachmentStoreOp {
        WHATEVER = 0,
        STORE,
        MAX
    };

    enum class AttachmentType {
        COLOR = 0,
        DEPTH_STENCIL,
        MAX
    };

    enum class VertexInputRate {
        VERTEX = 0,
        INSTANCE,
        MAX
    };

    enum class PrimitiveTopology {
        TRIANGLE_LIST = 0,
        MAX
    };

    enum class PolygonMode {
        FILL = 0,
        MAX
    };

    enum class CullMode {
        FRONT = 0,
        BACK,
        MAX
    };

    enum class FrontFace {
        CLOCKWISE = 0,
        MAX
    };

    enum class BlendFactor {
        ZERO,
        ONE,
        MAX
    };

    enum class BlendOp {
        ADD,
        MAX
    };

    enum class DynamicState {
        VIEWPORT = 0,
        LINE_WIDTH,
        MAX
    };

    enum class DescriptorType {
        UNIFORM_BUFFER = 0,
        STORAGE_BUFFER,
        SAMPLER,
        MAX
    };
}

namespace RHI {
    using Flags = uint64_t;

    enum class BufferUsageBits {
        NONE = 0x1,
        TRANSFER_SRC = 0x2,
        TRANSFER_DST = 0x4,
        VERTEX = 0x8,
        INDEX = 0x10,
        MAX
    };
    using BufferUsageFlags = Flags;

    enum class ImageUsageBits {
        NONE = 0x1,
        TRANSFER_SRC = 0x2,
        TRANSFER_DST = 0x4,
        SAMPLED = 0x8,
        MAX
    };
    using ImageUsageFlags = Flags;

    enum class ImageAspectBits {
        NONE = 0x1,
        COLOR = 0x2,
        DEPTH = 0x4,
        STENCIL = 0x8,
        MAX
    };
    using ImageAspectFlags = Flags;

    enum class ShaderCompileBits {
        NONE = 0x1,
        DEBUG = 0x2,
        NO_OPT = 0x4,
        MAX
    };
    using ShaderCompileFlags = Flags;

    enum class ImageLayoutBits {
        UNDEFINED = 0x1,
        PRESENT_SRC = 0x2,
        TRANSFER_DST = 0x4,
        COLOR_ATTACHMENT = 0x8,
        MAX
    };
    using ImageLayoutFlags = Flags;

    enum class ColorComponentBits {
        R = 0x1,
        G = 0x2,
        B = 0x4,
        A = 0x8,
        MAX
    };
    using ColorComponentFlags = Flags;

    enum class CommandBufferLevel {
        PRIMARY = 0,
        MAX
    };

    template <typename... E>
    Flags CombineBits(E&&... e)
    {
        Flags result;
        std::initializer_list<int> { (result |= static_cast<Flags>(e), 0)... };
        return result;
    }
}

namespace RHI {
    struct Extent2D {
        size_t x;
        size_t y;
    };

    struct Extent3D {
        size_t x;
        size_t y;
        size_t z;
    };

    struct Viewport {
        float x;
        float y;
        float width;
        float height;
        float minDepth;
        float maxDepth;
    };

    struct Scissor {
        Extent2D offset;
        Extent2D extent;
    };
}

#endif //EXPLOSION_RHI_ENUM_H
