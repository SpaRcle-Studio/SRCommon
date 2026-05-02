//
// Created by Monika on 20.01.2023.
//

#ifndef SR_ENGINE_RESOURCECONTAINER_H
#define SR_ENGINE_RESOURCECONTAINER_H

#include <Utils/Types/SharedPtr.h>

namespace SR_UTILS_NS {
    class SR_COMMON_DLL_API ResourceContainer : public SR_HTYPES_NS::SharedPtr<ResourceContainer> {
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<ResourceContainer>;

    public:
        ResourceContainer();
        ~ResourceContainer() override;

    public:
        void AddDependency(const ResourceContainer::Ptr& pContainer);
        void RemoveDependency(const ResourceContainer::Ptr& pContainer);

        SR_NODISCARD const std::unordered_set<ResourceContainer::Ptr>& GetContainerParents() const { return m_parents; }

    protected:
        void UpdateResources(int32_t depth = 0);
        virtual void OnResourceUpdated(ResourceContainer* pContainer, int32_t depth);

    private:
        std::unordered_set<ResourceContainer::Ptr> m_parents;
        std::unordered_set<ResourceContainer::Ptr> m_dependencies;

    };
}

#endif //SR_ENGINE_RESOURCECONTAINER_H
