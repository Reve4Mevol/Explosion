//
// Created by johnk on 11/1/2022.
//

#pragma once

#include <string>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <unordered_map>

#include <clipp.h>
#include <GLFW/glfw3.h>
#if PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif PLATFORM_MACOS
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>

#include <Common/Utility.h>
#include <Common/Debug.h>
#include <Common/String.h>
#include <Common/File.h>
#include <Common/Math/Matrix.h>
#include <Common/Math/Transform.h>
#include <Common/Math/Projection.h>
#include <Common/Math/View.h>
#include <RHI/RHI.h>
#include <Render/ShaderCompiler.h>
using namespace Common;

class Application {
public:
    NonCopyable(Application)
    explicit Application(std::string n) : rhiType(RHI::RHIType::vulkan), window(nullptr), name(std::move(n)), width(1024), height(768) {}

    virtual ~Application() = default;

    int Run(int argc, char* argv[])
    {
        std::string rhiString;
        auto cli = (
            clipp::option("-w").doc("window width, 1024 by default") & clipp::value("width", width),
            clipp::option("-h").doc("window height, 768 by default") & clipp::value("height", height),
            clipp::required("-rhi").doc("RHI type, can be 'DirectX12', 'Metal' or 'Vulkan'") & clipp::value("RHI type", rhiString)
        );
        if (!clipp::parse(argc, argv, cli)) {
            std::cout << clipp::make_man_page(cli, argv[0]);
            return -1;
        }
        static const std::unordered_map<std::string, RHI::RHIType> RHI_MAP = {
            { "DirectX12", RHI::RHIType::directX12 },
            { "Vulkan", RHI::RHIType::vulkan },
            { "Metal", RHI::RHIType::metal }
        };
        auto iter = RHI_MAP.find(rhiString);
        rhiType = iter == RHI_MAP.end() ? RHI::RHIType::directX12 : iter->second;

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), name.c_str(), nullptr, nullptr);
        OnCreate();
        while (!glfwWindowShouldClose(window)) {
            OnDrawFrame();
            glfwPollEvents();
        }
        OnDestroy();

        glfwTerminate();
        return 0;
    }

protected:
    virtual void OnStart() {}
    virtual void OnCreate() {}
    virtual void OnDestroy() {}
    virtual void OnDrawFrame() {}

    void* GetPlatformWindow()
    {
#if PLATFORM_WINDOWS
        return glfwGetWin32Window(window);
#elif PLATFORM_MACOS
        return glfwGetCocoaWindow(window);
#else
        Assert(false);
        return nullptr;
#endif
    }

    void CompileShader(std::vector<uint8_t>& byteCode, const std::string& fileName, const std::string& entryPoint, RHI::ShaderStageBits shaderStage, std::vector<std::string> includePaths = {})
    {
        std::string shaderSource = Common::FileUtils::ReadTextFile(fileName);

        Render::ShaderCompileInput info;
        info.source = shaderSource;
        info.entryPoint = entryPoint;
        info.stage = shaderStage;
        Render::ShaderCompileOptions options;
        if (!includePaths.empty()) {
            options.includePaths.insert(options.includePaths.end(), includePaths.begin(), includePaths.end());
        }
        if (rhiType == RHI::RHIType::directX12) {
            options.byteCodeType = Render::ShaderByteCodeType::dxil;
        } else if (rhiType == RHI::RHIType::vulkan) {
            options.byteCodeType = Render::ShaderByteCodeType::spirv;
        } else if (rhiType == RHI::RHIType::metal) {
            options.byteCodeType = Render::ShaderByteCodeType::mbc;
        }
        options.withDebugInfo = false;
        auto future = Render::ShaderCompiler::Get().Compile(info, options);

        future.wait();
        auto result = future.get();
        if (!result.success) {
            std::cout << "failed to compiler shader (" << fileName << ", " << info.entryPoint << ")" << std::endl << result.errorInfo << std::endl;
        }
        Assert(result.success);
        byteCode = result.byteCode;
    }

    RHI::RHIType rhiType;
    GLFWwindow* window;
    std::string name;
    uint32_t width;
    uint32_t height;
};
