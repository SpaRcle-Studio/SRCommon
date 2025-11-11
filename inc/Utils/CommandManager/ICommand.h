//
// Created by Monika on 08.01.2022.
//

#ifndef SR_ENGINE_ICOMMAND_H
#define SR_ENGINE_ICOMMAND_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/TypeTraits/TypeTraits.h>

namespace SR_UTILS_NS {
    class CmdManager;

    class SR_COMMON_DLL_API ICommand {
    public:
        inline static auto DATA_ID = SR_UTILS_NS::SerializationId::Create("command");

    public:
        ICommand() = default;
        virtual ~ICommand() = default;

    };
}

#endif //SR_ENGINE_ICOMMAND_H
