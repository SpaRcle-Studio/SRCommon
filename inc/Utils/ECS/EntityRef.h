//
// Created by Monika on 16.07.2025.
//

#ifndef SR_ENGINE_UTILS_ENTITY_REF_H
#define SR_ENGINE_UTILS_ENTITY_REF_H

#include <Utils/Serialization/Serializable.h>
#include <Utils/ECS/EntityController.h>

namespace SR_UTILS_NS {
    /// @inspector(EntityRefPropertyDrawer)
    class EntityRefBase : public SR_UTILS_NS::Serializable {
        using Super = SR_UTILS_NS::Serializable;
        SR_CLASS()
    public:
        EntityRefBase();

    public:
        SR_NODISCARD virtual StringAtom GetTypeName() const noexcept;
        SR_NODISCARD const Entity::Ptr& GetEntity() const noexcept;
        SR_NODISCARD EntityId GetEntityId() const noexcept { return m_entityId; }
        SR_NODISCARD bool IsResolved() const noexcept;

        void OnEntityIdReplaced(const EntityReplaceMap& replaceMap);
        void SetEntityId(EntityId entityId) noexcept;

        void Resolve() const noexcept;

        SR_NODISCARD operator bool() const noexcept;

    private:
        /// @property @hidden
        EntityId m_entityId = SR_ID_INVALID;
        mutable Entity::Ptr m_pEntity;

    };

    template<class T> class EntityRef : public EntityRefBase {
        using Super = EntityRefBase;
    public:
        SR_NODISCARD StringAtom GetTypeName() const noexcept override {
            if constexpr (!std::is_same_v<T, void>) {
                return T::GetClassStaticName();
            }
            else {
                return "void";
            }
        }

        SR_NODISCARD SR_HTYPES_NS::SharedPtr<T> Get() const noexcept {
            if constexpr (!std::is_same_v<T, void>) {
                return GetEntity().template StaticCast<T>();
            }
            else {
                return SR_HTYPES_NS::SharedPtr<T>();
            }
        }

        SR_NODISCARD T* GetRaw() const noexcept {
            if constexpr (!std::is_same_v<T, void>) {
                return const_cast<T*>(static_cast<const T*>(GetEntity().Get()));
            }
            return nullptr;
        }
    };
}

#endif //SR_ENGINE_UTILS_ENTITY_REF_H
