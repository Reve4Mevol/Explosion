//
// Created by johnk on 2022/1/24.
//

#ifndef EXPLOSION_RHI_DX12_BUFFER_H
#define EXPLOSION_RHI_DX12_BUFFER_H

#include <wrl/client.h>
#include <d3d12.h>

#include <RHI/Buffer.h>

using namespace Microsoft::WRL;

namespace RHI::DirectX12 {
    class DX12Device;

    class DX12Buffer : public Buffer {
    public:
        explicit DX12Buffer(DX12Device& device, const BufferCreateInfo* createInfo);
        ~DX12Buffer() override;

        void* Map(MapMode mapMode, size_t offset, size_t length) override;
        void UnMap() override;
        void Destroy() override;

        ComPtr<ID3D12Resource>& GetDX12Resource();
        D3D12_CONSTANT_BUFFER_VIEW_DESC* GetDX12CBVDesc();
        D3D12_UNORDERED_ACCESS_VIEW_DESC* GetDX12UAVDesc();

    private:
        void CreateDX12Buffer(DX12Device& device, const BufferCreateInfo* createInfo);
        void CreateDX12ViewDesc(const BufferCreateInfo* createInfo);

        MapMode mapMode;
        ComPtr<ID3D12Resource> dx12Resource;
        std::unique_ptr<D3D12_CONSTANT_BUFFER_VIEW_DESC> dx12CBVDesc;
        std::unique_ptr<D3D12_UNORDERED_ACCESS_VIEW_DESC> dx12UAVDesc;
    };
}

#endif //EXPLOSION_RHI_DX12_BUFFER_H
