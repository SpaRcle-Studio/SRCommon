//
// Created by Monika on 01.01.2023.
//

#ifndef SR_ENGINE_MIGRATION_MANAGER_H
#define SR_ENGINE_MIGRATION_MANAGER_H

#include <Utils/Common/Singleton.h>
#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS {
    class IDeserializer;
    class Serializable;

    class MigrationManager : public Singleton<MigrationManager> {
        SR_REGISTER_SINGLETON(MigrationManager)
        using Version = uint64_t;
        struct MigrationInfo {
            Version from;
            Version to;
            StringAtom factory;

            bool operator<(const MigrationInfo& other) const {
                return std::min(from, to) < std::min(other.from, other.to);
            }
        };
    public:
        SR_NODISCARD MigrationResult Migrate(SR_UTILS_NS::IDeserializer& deserializer, SR_UTILS_NS::Serializable& obj, Version from, Version to);

    private:
        void Initialize();

    private:
        bool m_isInitialized = false;
        std::map<StringAtom, std::vector<MigrationInfo>> m_migrations;

    };
}

#endif //SR_ENGINE_MIGRATION_MANAGER_H
