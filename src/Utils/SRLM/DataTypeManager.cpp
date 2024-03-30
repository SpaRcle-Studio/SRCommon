//
// Created by Monika on 26.08.2023.
//

#include <Utils/SRLM/DataTypeManager.h>
#include <Utils/SRLM/DataType.h>
#include <Utils/Common/HashManager.h>
#include <Utils/Resources/ResourceManager.h>
#include <Utils/Xml.h>

namespace SR_SRLM_NS {
    DataType* DataTypeManager::CreateByName(const std::string& name) {
        return CreateByName(SR_HASH_STR_REGISTER(name));
    }

    DataType* DataTypeManager::CreateByName(uint64_t hashName) {
        SR_LOCK_GUARD;

        if (auto&& pIt = m_structs.find(hashName); pIt != m_structs.end()) {
            return pIt->second->Copy();
        }

        if (auto&& pData = DataTypeAllocator::Instance().Allocate(hashName)) {
            return pData;
        }

        if (auto&& pReflector = SR_UTILS_NS::EnumReflectorManager::Instance().GetReflector(hashName)) {
            return new DataTypeEnum(0, pReflector);
        }

        if (auto&& stringName = SR_UTILS_NS::HashManager::Instance().HashToString(hashName); !stringName.empty()) {
            SR_ERROR("DataTypeManager::CreateByName() : failed to create \"" + stringName + "\"!");
        }
        else {
            SRHalt("Type by hash \"" + SR_UTILS_NS::ToString(hashName) + "\" not found!");
        }

        return nullptr;
    }

    void DataTypeManager::Clear() {
        SR_LOCK_GUARD;

        for (auto&& [hash, pData] : m_structs) {
            delete pData;
        }
        m_structs.clear();
    }

    void DataTypeManager::ReloadSettings() {
        SR_LOCK_GUARD;

        auto&& xmlDocument = SR_XML_NS::Document::Load(m_watcher->GetPath());
        if (!xmlDocument.Valid()) {
            SR_ERROR("DataTypeManager::ReloadSettings() : failed to read xml!\n\tPath: " + m_watcher->GetPath().ToStringRef());
            return;
        }

        auto&& xmlRoot = xmlDocument.Root().GetNode("SRLM");

        for (auto&& xmlStruct : xmlRoot.TryGetNodes("Struct")) {
            auto&& structName = xmlStruct.GetAttribute("Name").ToString();
            auto&& structHash = SR_UTILS_NS::HashManager::Instance().AddHash(structName);

            auto&& pStruct = m_structs[structHash] = new DataTypeStruct(structHash);

            for (auto&& xmlVar : xmlStruct.GetNodes()) {
                auto&& varName = xmlVar.GetAttribute("Name");

                auto&& nameHash = SR_UTILS_NS::HashManager::Instance().AddHash(varName.ToString());
                auto&& typeHash = SR_UTILS_NS::HashManager::Instance().AddHash(xmlVar.Name());

                if (auto&& pData = CreateByName(typeHash)) {
                    pStruct->AddVariable(nameHash, pData);
                }
                else {
                    SRHalt("Type \"" + xmlVar.Name() + "\" not found!");
                }
            }
        }
    }

    void DataTypeManager::OnSingletonDestroy() {
        Singleton::OnSingletonDestroy();

        if (m_watcher) {
            m_watcher->Stop();
        }
    }

    void DataTypeManager::InitSingleton() {
        Singleton::InitSingleton();

        m_watcher = SR_UTILS_NS::ResourceManager::Instance().StartWatch(
                SR_UTILS_NS::ResourceManager::Instance().GetResPath().Concat("Engine/Configs/SRLMTypes.xml")
        );

        ReloadSettings();

        m_watcher->SetCallBack([this](FileWatcher* pWatcher) {
            ReloadSettings();
        });
    }

    const DataTypeStruct* DataTypeManager::GetStruct(DataTypeManager::Hash hashName) const {
        SR_LOCK_GUARD;

        if (auto&& pIt = m_structs.find(hashName); pIt != m_structs.end()) {
            return pIt->second;
        }

        return nullptr;
    }

    bool DataTypeManager::IsStructExists(DataTypeManager::Hash hashName) const {
        SR_LOCK_GUARD;

        return m_structs.count(hashName) == 1;
    }
}