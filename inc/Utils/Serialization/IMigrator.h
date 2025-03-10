//
// Created by Monika on 08.03.2025.
//

#ifndef SR_ENGINE_I_MIGRATOR_H
#define SR_ENGINE_I_MIGRATOR_H

#include <Utils/TypeTraits/SRClass.h>
#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS {
    class IDeserializer;

    class IMigrator : public SRClass, public SR_HTYPES_NS::SharedPtr<IMigrator> {
        using Super = SR_HTYPES_NS::SharedPtr<IMigrator>;
        SR_CLASS()
    public:
        IMigrator()
            : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
        { }

    public:
        SR_NODISCARD virtual MigrationResult Migrate(SR_UTILS_NS::SerializationNode& node) = 0;

    };

    /**
    Migrator example:

        class Migrator_SceneObject_from_20150200_to_20150301 final : public IMigrator {
            SR_CLASS()
        public:
            SR_NODISCARD MigrationResult Migrate(SR_UTILS_NS::SerializationNode& node) final;

        };
    */
}

#endif //SR_ENGINE_I_MIGRATOR_H
