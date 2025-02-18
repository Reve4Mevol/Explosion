//
// Created by johnk on 2025/2/18.
//

#include <Runtime/Viewport.h>

namespace Runtime {
    Viewport::~Viewport() = default;

    Viewport::Viewport() = default;

    PresentInfo::PresentInfo()
        : backBuffer(nullptr)
        , imageReadySemaphore(nullptr)
        , renderFinishedSemaphore(nullptr)
    {
    }
}
