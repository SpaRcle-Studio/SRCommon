//
// Created by Monika on 16.07.2025.
//

#ifndef SR_ENGINE_UTILS_ENTITY_REF_H
#define SR_ENGINE_UTILS_ENTITY_REF_H

#include <Utils/Serialization/Serializable.h>
#include <Utils/TypeTraits/Factory.h>
#include <Utils/ECS/Entity.h>

namespace SR_UTILS_NS {
    /// @inspector(EntityRefPropertyDrawer)
    class EntityRefBase : public SR_UTILS_NS::Serializable {
        using Super = SR_UTILS_NS::Serializable;
        SR_CLASS()
    public:
        EntityRefBase();

    public:
        /// @method @evaluate
        SR_NODISCARD virtual StringAtom GetTypeName() const noexcept;
        /// @method @evaluate
        SR_NODISCARD Entity::Ptr GetEntity() const noexcept;
        /// @method @evaluate
        SR_NODISCARD EntityId GetEntityId() const noexcept { return m_entityId; }

        SR_NODISCARD bool IsResolved() const noexcept;

        SR_NODISCARD virtual bool IsApplicable(const SR_HTYPES_NS::SharedPtr<Entity>& pEntity) const noexcept;

        void OnEntityIdReplaced(const EntityReplaceMap& replaceMap);
        void SetEntityId(EntityId entityId) noexcept;

        void Resolve() const noexcept;

        SR_NODISCARD operator bool() const noexcept;

    private:
        /// @property @hidden
        EntityId m_entityId = SR_ID_INVALID;
        mutable Entity::Ptr m_pEntity;

    };

    template<typename T> SR_NODISCARD StringAtom EntityRefExtractTypeName() noexcept {
        /// @note both branches must return the same name: Factory::Register() stores the meta factory name
        ///       under typeid(T*), and it is the very same name that GetClassStaticName() returns. This is what
        ///       makes the completeness check here safe - the compiler may answer it differently in different
        ///       translation units, but the resulting name does not change. Do not add a branch that changes
        ///       observable behaviour, see IsCompleteType.
        if constexpr (!std::is_same_v<T, void>) {
            if constexpr (SR_UTILS_NS::IsCompleteTypeV<T>) {
                return T::GetClassStaticName();
            }
            else {
                return Factory::Instance().GetNameByTypeId<T>();
            }
        }
        static const StringAtom voidTypeName("void");
        return voidTypeName;
    }

    template<class T> class EntityRef : public EntityRefBase {
        using Super = EntityRefBase;
    public:
        SR_NODISCARD StringAtom GetTypeName() const noexcept override {
            return SR_UTILS_NS::EntityRefExtractTypeName<T>();
        }

        SR_NODISCARD SR_HTYPES_NS::SharedPtr<T> Get() const noexcept {
            if constexpr (!std::is_same_v<T, void>) {
                return SR_UTILS_NS::StaticPointerCast<T>(GetEntity());
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

    /// @inspector(EntityRefPropertyDrawer)
    class EntityRefDynamic : public EntityRefBase {
        using Super = EntityRefBase;
        SR_CLASS()
    public:
        SR_NODISCARD StringAtom GetTypeName() const noexcept override {
            return m_typeName;
        }

        void SetTypeName(StringAtom typeName) { m_typeName = typeName; }

    private:
        /// @property
        StringAtom m_typeName;

    };
}

#endif //SR_ENGINE_UTILS_ENTITY_REF_H
