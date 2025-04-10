//
// Created by johnk on 2025/2/19.
//

#pragma once

#include <Runtime/Meta.h>
#include <Runtime/ECS.h>
#include <Runtime/Asset/Asset.h>
#include <Runtime/Api.h>

namespace Runtime {
    struct RUNTIME_API EClass() Level final : Asset {
        EPolyClassBody(Level)

        explicit Level(Core::Uri inUri);
        ~Level() override;

        EProperty() ECArchive archive;
    };
}
