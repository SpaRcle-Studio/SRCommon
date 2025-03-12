//
// Created by Monika on 01.01.2023.
//

#include <Utils/Serialization/MigrationManager.h>
#include <Utils/Serialization/IMigrator.h>
#include <Utils/Serialization/Serializable.h>
#include <Utils/TypeTraits/Factory.h>

namespace SR_UTILS_NS {
    MigrationResult MigrationManager::Migrate(IDeserializer& deserializer, SR_UTILS_NS::Serializable& obj, Version from, Version to) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        Initialize();

        auto&& pBaseSerialization = dynamic_cast<IBaseSerialization*>(&deserializer);
        if (!pBaseSerialization) {
            SR_ERROR("MigrationManager::Migrate() : deserializer is not IBaseSerialization!");
            return MigrationResult::Fatal;
        }

        StringAtom factoryName = obj.GetMeta()->GetFactoryName();

        auto&& pMigratorsIt = m_migrations.find(factoryName);
        if (pMigratorsIt == m_migrations.end()) {
            SR_ERROR("MigrationManager::Migrate() : no migrators for object \"{}\"!", factoryName);
            return MigrationResult::Unhandled;
        }

        auto&& migrationList = pMigratorsIt->second;

        Version currentVersion = from;

        auto&& factory = SR_UTILS_NS::Factory::Instance();

        while (currentVersion != to) {
            auto&& pIt = std::find_if(migrationList.begin(), migrationList.end(),
               [currentVersion, to, from](const MigrationInfo& m) {
                   return
                       // Если есть прямая миграция
                       (m.from == currentVersion && m.to == to) ||
                       // Следующий шаг в нужную сторону
                       (m.from == currentVersion && (to > from ? m.to > currentVersion : m.to < currentVersion));
               });

            if (pIt == migrationList.end()) {
                SR_ERROR("MigrationManager::Migrate() : no valid migration path from {} to {} for \"{}\"!", from, to, factoryName);
                return MigrationResult::Unhandled;
            }

            StringAtom migratorName = pIt->factory;
            SR_LOG("MigrationManager::Migrate() : applying migrator \"{}\" from {} to {}", migratorName, pIt->from, pIt->to);

            auto&& pMigrator = factory.Create<IMigrator>(migratorName);
            if (!pMigrator) {
                SR_ERROR("MigrationManager::Migrate() : failed to create migrator \"{}\"!", migratorName);
                return MigrationResult::Unhandled;
            }

            const MigrationResult result = pMigrator->Migrate(pBaseSerialization->GetWalkNode());
            if (result != MigrationResult::Success) {
                SR_ERROR("MigrationManager::Migrate() : failed to migrate \"{}\" from {} to {}!", factoryName, pIt->from, pIt->to);
                return result;
            }

            currentVersion = pIt->to;
        }

        return MigrationResult::Success;
    }

    void MigrationManager::Initialize() {
        if (m_isInitialized) {
            return;
        }

        SR_TRACY_ZONE;

        SR_INFO("MigrationManager::Initialize() : register migrators...");

        m_isInitialized = true;

        auto&& factory = SR_UTILS_NS::Factory::Instance();
        auto&& inheritances = factory.GetInheritances(IMigrator::GetClassStaticName());

        static std::regex pattern(R"(^Migrator_([A-Za-z0-9_]+)_from_([0-9]+)_to_([0-9]+)$)");

        m_migrations.clear();

        for (auto&& migratorName : inheritances) {
            if (factory.IsAbstract(migratorName)) {
                continue;
            }
 
            std::smatch match;
            if (std::regex_match(migratorName.ToStringRef(), match, pattern)) {
                const SR_UTILS_NS::StringAtom objectName = match[1].str();
                const auto fromVersion = SR_UTILS_NS::LexicalCast<uint64_t>(match[2].str());
                const auto toVersion = SR_UTILS_NS::LexicalCast<uint64_t>(match[3].str());

                MigrationInfo info;
                info.factory = migratorName;
                info.from = fromVersion;
                info.to = toVersion;
                m_migrations[objectName].emplace_back(info);

                SR_LOG("MigrationManager::Initialize() : register migrator \"{}\" for object \"{}\" from {} to {}", migratorName, objectName, fromVersion, toVersion);
            }
            else {
                SR_ERROR("MigrationManager::Initialize() : invalid migrator class name format! \"{}\"", migratorName);
                continue;
            }
        }

        for (auto& [key, migrationList] : m_migrations) {
            std::sort(migrationList.begin(), migrationList.end());

            for (size_t i = 1; i < migrationList.size(); ++i) {
                const Version prevTo = migrationList[i - 1].to;
                const Version currFrom = migrationList[i].from;

                if (prevTo != currFrom) {
                    SR_ERROR("MigrationManager::Initialize() : migration chain for {} is broken at version {} -> {}", prevTo, currFrom);
                }
            }
        }

        SR_INFO("MigrationManager::Initialize() : {} migrators registered!", m_migrations.size());
    }
}