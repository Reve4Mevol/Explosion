//
// Created by johnk on 2022/1/24.
//

#include <vector>
#include <functional>

#include <directx/d3dx12.h>

#include <RHI/Enum.h>
#include <RHI/DirectX12/Common.h>
#include <RHI/DirectX12/Device.h>
#include <RHI/DirectX12/Buffer.h>

namespace RHI::DirectX12 {
    static D3D12_HEAP_TYPE GetDX12HeapType(BufferUsageFlags bufferUsages)
    {
        static std::unordered_map<BufferUsageFlags, D3D12_HEAP_TYPE> rules = {
            { BufferUsageBits::MAP_WRITE | BufferUsageBits::COPY_SRC, D3D12_HEAP_TYPE_UPLOAD },
            { BufferUsageBits::MAP_READ | BufferUsageBits::COPY_DST, D3D12_HEAP_TYPE_READBACK }
            // TODO check other conditions ?
        };
        static D3D12_HEAP_TYPE fallback = D3D12_HEAP_TYPE_DEFAULT;

        for (const auto& rule : rules) {
            if (bufferUsages & rule.first) {
                return rule.second;
            }
        }
        return fallback;
    }

    static D3D12_RESOURCE_STATES GetDX12ResourceStates(BufferUsageFlags bufferUsages)
    {
        static std::unordered_map<BufferUsageBits, D3D12_RESOURCE_STATES> rules = {
            { BufferUsageBits::COPY_SRC, D3D12_RESOURCE_STATE_COPY_SOURCE },
            { BufferUsageBits::COPY_DST, D3D12_RESOURCE_STATE_COPY_DEST },
            { BufferUsageBits::INDEX, D3D12_RESOURCE_STATE_GENERIC_READ },
            { BufferUsageBits::VERTEX, D3D12_RESOURCE_STATE_GENERIC_READ },
            { BufferUsageBits::UNIFORM, D3D12_RESOURCE_STATE_GENERIC_READ },
            { BufferUsageBits::STORAGE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
            { BufferUsageBits::INDIRECT, D3D12_RESOURCE_STATE_GENERIC_READ },
            // TODO check other conditions ?
        };

        D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;
        for (const auto& rule : rules) {
            if (bufferUsages & rule.first) {
                result |= rule.second;
            }
        }
        return result;
    }

    static MapMode GetMapMode(BufferUsageFlags bufferUsages)
    {
        static std::unordered_map<BufferUsageBits, MapMode> rules = {
            { BufferUsageBits::MAP_READ, MapMode::READ },
            { BufferUsageBits::MAP_WRITE, MapMode::WRITE }
        };

        for (const auto& rule : rules) {
            if (bufferUsages & rule.first) {
                return rule.second;
            }
        }
        return MapMode::READ;
    }

    static inline bool IsConstantBuffer(BufferUsageFlags bufferUsages)
    {
        return bufferUsages & BufferUsageBits::UNIFORM;
    }

    static inline bool IsUnorderedAccessBuffer (BufferUsageFlags bufferUsages)
    {
        return bufferUsages & BufferUsageBits::STORAGE;
    }
}

namespace RHI::DirectX12 {
    DX12Buffer::DX12Buffer(DX12Device& device, const BufferCreateInfo* createInfo) : Buffer(createInfo), mapMode(GetMapMode(createInfo->usages)), dx12CpuDescriptorHandle()
    {
        CreateDX12Buffer(device, createInfo);
        CreateDX12Descriptor(device, createInfo);
    }

    DX12Buffer::~DX12Buffer() = default;

    void* DX12Buffer::Map(MapMode mode, size_t offset, size_t length)
    {
        if (mapMode != mode) {
            throw DX12Exception("unsuitable map mode");
        }

        void* data;
        CD3DX12_RANGE range(offset, offset + length);
        if (FAILED(dx12Resource->Map(0, &range, &data))) {
            throw DX12Exception("failed to map dx12 buffer");
        }
        return data;
    }

    void DX12Buffer::UnMap()
    {
        dx12Resource->Unmap(0, nullptr);
    }

    void DX12Buffer::Destroy()
    {
        delete this;
    }

    ComPtr<ID3D12Resource>& DX12Buffer::GetDX12Resource()
    {
        return dx12Resource;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE DX12Buffer::GetDX12CpuDescriptorHandle()
    {
        return dx12CpuDescriptorHandle;
    }

    void DX12Buffer::CreateDX12Buffer(DX12Device& device, const BufferCreateInfo* createInfo)
    {
        CD3DX12_HEAP_PROPERTIES heapProperties(GetDX12HeapType(createInfo->usages));
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(createInfo->size);

        if (FAILED(device.GetDX12Device()->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            GetDX12ResourceStates(createInfo->usages),
            nullptr,
            IID_PPV_ARGS(&dx12Resource)
        ))) {
            throw DX12Exception("failed to create dx12 buffer");
        }
    }

    void DX12Buffer::CreateDX12Descriptor(DX12Device& device, const BufferCreateInfo* createInfo)
    {
        if (IsConstantBuffer(createInfo->usages)) {
            D3D12_CONSTANT_BUFFER_VIEW_DESC desc {};
            desc.BufferLocation = dx12Resource->GetGPUVirtualAddress();
            desc.SizeInBytes = createInfo->size;
            dx12CpuDescriptorHandle = device.AllocateCbvSrvUavDescriptor();
            device.GetDX12Device()->CreateConstantBufferView(&desc, dx12CpuDescriptorHandle);
        } else if (IsUnorderedAccessBuffer(createInfo->usages)) {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc {};
            desc.Format = DXGI_FORMAT_UNKNOWN;
            desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            desc.Buffer.FirstElement = 0;
            desc.Buffer.NumElements = createInfo->size;
            dx12CpuDescriptorHandle = device.AllocateCbvSrvUavDescriptor();
            device.GetDX12Device()->CreateUnorderedAccessView(dx12Resource.Get(), nullptr, &desc, dx12CpuDescriptorHandle);
        }
    }
}
