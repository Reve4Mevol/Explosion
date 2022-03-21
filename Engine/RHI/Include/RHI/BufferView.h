//
// Created by johnk on 20/3/2022.
//

#pragma once

#include <cstddef>

#include <Common/Utility.h>

namespace RHI {
    struct BufferViewCreateInfo {
        size_t offset;
        size_t size;
    };

    class BufferView {
    public:
        NON_COPYABLE(BufferView)
        virtual ~BufferView();

        virtual void Destroy() = 0;

    protected:
        explicit BufferView(const BufferViewCreateInfo* createInfo);
    };
}
