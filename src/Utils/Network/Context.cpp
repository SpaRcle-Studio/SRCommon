//
// Created by Monika on 20.02.2024.
//

#include <Utils/Network/Context.h>

#include <Utils/Network/Asio/AsioContext.h>

namespace SR_NETWORK_NS {
    Context::Context()
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
    { }

    SR_HTYPES_NS::SharedPtr<Context> Context::Create() {
        return AsioContext::MakeShared<AsioContext, Context>();
    }
}
