//
// Created by johnk on 22/3/2022.
//

#include <RHI/DirectX12/CommandBuffer.h>
#include <RHI/DirectX12/Common.h>
#include <RHI/DirectX12/Device.h>
#include <RHI/DirectX12/CommandRecorder.h>
#include <RHI/DirectX12/BindGroup.h>

namespace RHI::DirectX12 {
    RuntimeDescriptorHeap::RuntimeDescriptorHeap(DX12Device& inDevice, const D3D12_DESCRIPTOR_HEAP_TYPE inHeapType, const uint32_t inCapacity)
        : device(inDevice)
        , capacity(inCapacity)
        , used(false)
        , nativeHeapType(inHeapType)
    {
        descriptorIncrementSize = device.GetNative()->GetDescriptorHandleIncrementSize(inHeapType);

        D3D12_DESCRIPTOR_HEAP_DESC desc {};
        desc.NumDescriptors = capacity;
        desc.Type = nativeHeapType;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        Assert(SUCCEEDED(device.GetNative()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&nativeDescriptorHeap))));
    }

    RuntimeDescriptorHeap::~RuntimeDescriptorHeap() = default;

    void RuntimeDescriptorHeap::ResetUsed()
    {
        used = 0;
    }

    ID3D12DescriptorHeap* RuntimeDescriptorHeap::GetNative() const
    {
        return nativeDescriptorHeap.Get();
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE RuntimeDescriptorHeap::NewGpuDescriptorHandle(const CD3DX12_CPU_DESCRIPTOR_HANDLE inCpuHandle)
    {
        Assert(used + 1 < capacity);

        const uint32_t srcDescriptorNum = 1; // NOLINT
        const CD3DX12_CPU_DESCRIPTOR_HANDLE srcDescriptorStart = inCpuHandle;

        const uint32_t destDescriptorNum = 1; // NOLINT
        CD3DX12_CPU_DESCRIPTOR_HANDLE destDescriptorStart(nativeDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        destDescriptorStart.Offset(static_cast<int32_t>(used), descriptorIncrementSize);

        device.GetNative()->CopyDescriptors(1, &destDescriptorStart, &destDescriptorNum, 1, &srcDescriptorStart, &srcDescriptorNum, nativeHeapType);
        CD3DX12_GPU_DESCRIPTOR_HANDLE result(nativeDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
        result.Offset(static_cast<int32_t>(used), descriptorIncrementSize);
        used++;
        return result;
    }

    RuntimeDescriptorCompact::RuntimeDescriptorCompact(DX12Device& inDevice)
        : device(inDevice)
    {
        cbvSrvUavHeap = Common::MakeUnique<RuntimeDescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cbvSrvUavHeapCapacity);
        samplerHeap = Common::MakeUnique<RuntimeDescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, samplerHeapCapacity);
    }

    void RuntimeDescriptorCompact::ResetUsed() const
    {
        cbvSrvUavHeap->ResetUsed();
        samplerHeap->ResetUsed();
    }

    std::vector<ID3D12DescriptorHeap*> RuntimeDescriptorCompact::GetNative() const
    {
        return {cbvSrvUavHeap->GetNative(), samplerHeap->GetNative() };
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE RuntimeDescriptorCompact::NewGpuDescriptorHandle(const HlslBindingRangeType inRangeType, const CD3DX12_CPU_DESCRIPTOR_HANDLE inCpuHandle) const
    {
        if (inRangeType == HlslBindingRangeType::sampler) {
            return samplerHeap->NewGpuDescriptorHandle(inCpuHandle);
        } else {
            return cbvSrvUavHeap->NewGpuDescriptorHandle(inCpuHandle);
        }
    }

    DX12CommandBuffer::DX12CommandBuffer(DX12Device& inDevice)
        : device(inDevice)
    {
        CreateNativeCommandAllocator(inDevice);
        CreateNativeCommandList(inDevice);
        runtimeDescriptorHeaps = Common::MakeUnique<RuntimeDescriptorCompact>(inDevice);
    }

    DX12CommandBuffer::~DX12CommandBuffer() = default;

    Common::UniquePtr<CommandRecorder> DX12CommandBuffer::Begin()
    {
        return Common::UniquePtr<CommandRecorder>(new DX12CommandRecorder(device, *this));
    }

    ID3D12CommandAllocator* DX12CommandBuffer::GetNativeAllocator() const
    {
        return nativeCommandAllocator.Get();
    }

    ID3D12GraphicsCommandList* DX12CommandBuffer::GetNativeCmdList() const
    {
        return nativeGraphicsCommandList.Get();
    }

    RuntimeDescriptorCompact* DX12CommandBuffer::GetRuntimeDescriptorHeaps() const
    {
        return runtimeDescriptorHeaps.Get();
    }

    void DX12CommandBuffer::CreateNativeCommandAllocator(DX12Device& inDevice)
    {
        Assert(SUCCEEDED(inDevice.GetNative()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&nativeCommandAllocator))));
    }

    void DX12CommandBuffer::CreateNativeCommandList(DX12Device& inDevice)
    {
        Assert(SUCCEEDED(inDevice.GetNative()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, nativeCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&nativeGraphicsCommandList))));
        Assert(SUCCEEDED(nativeGraphicsCommandList->Close()));
    }
}
