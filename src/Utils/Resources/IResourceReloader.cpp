//
// Created by Monika on 11.03.2023.
//

#include <Utils/Resources/IResourceReloader.h>
#include <Utils/Resources/ResourceInfo.h>
#include <Utils/Events/Broadcaster.h>
#include <Utils/Common/SubscriptionMessage.h>

namespace SR_UTILS_NS {
    bool DefaultResourceReloader::Reload(const SR_UTILS_NS::Path& /** path */, ResourcesStorage* pStorage) {
        SR_TRACY_ZONE;

        pStorage->ForEach([this](IResource& resource) {
            if (IsResourceSuitableForReload(resource)) {
                resource.Reload();
            }
        });

        SR_UTILS_NS::SubscriptionMessage msg;
        msg.SetString("Id", pStorage->id);
        SR_UTILS_NS::Broadcaster::Instance().Broadcast(Events::EVENT_ON_RESOURCE_RELOADED_ID, msg);

        return true;
    }

    bool IResourceReloader::IsResourceSuitableForReload(const IResource& resource) const {
        SR_TRACY_ZONE;

        if (resource.IsDestroyed()) {
            return false;
        }

        auto&& loadState = resource.GetResourceLoadState();

        using LS = IResource::LoadState;
        if (loadState == LS::Reloading || loadState == LS::Loading || loadState == LS::Unloading) {
            return false;
        }

        SR_NOOP;

        return true;
    }
}
