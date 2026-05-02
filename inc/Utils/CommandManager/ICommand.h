//
// Created by Monika on 08.01.2022.
//

#ifndef SR_ENGINE_ICOMMAND_H
#define SR_ENGINE_ICOMMAND_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/TypeTraits/TypeTraits.h>

namespace SR_UTILS_NS {
    class CmdManager;

    SR_CONSTEXPR SerializationId COMMAND_DATA_ID = SerializationId::Create("ICommand");

    class SR_COMMON_DLL_API ICommand {
    public:
        ICommand() = default;
        virtual ~ICommand() = default;

    };
}

#endif //SR_ENGINE_ICOMMAND_H
