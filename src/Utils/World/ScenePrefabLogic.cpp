//
// Created by Monika on 24.12.2022.
//

#include <Utils/World/ScenePrefabLogic.h>
#include <Utils/World/Scene.h>
#include <Utils/ECS/Transform3D.h>
#include <Utils/ECS/LayerManager.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/Serialization/SerializationFlags.h>

#include <Codegen/ScenePrefabLogic.generated.hpp>

namespace SR_WORLD_NS {
    ScenePrefabLogic::~ScenePrefabLogic() {
        m_pSOCustomData.reset();
    }

    bool ScenePrefabLogic::SaveSOAsPrefab(ISerializer& serializer, const SR_HTYPES_NS::SharedPtr<SceneObject>& pSO) {
        if (!pSO) {
            SRHalt("ScenePrefabLogic::SaveSOAsPrefab() : pSO is nullptr!");
            return false;
        }

        serializer.BeginObject(SR_UTILS_NS::SerializationId::Create("info"));
        {
            serializer.WriteString(pSO->GetMeta()->GetFactoryName(), SR_UTILS_NS::SerializationId::Create("type"));
        }
        serializer.EndObject();

        serializer.BeginObject(SR_UTILS_NS::SerializationId::Create("data"));
        {
            pSO->Save(serializer);
        }
        serializer.EndObject();

        return true;
    }

    bool ScenePrefabLogic::LoadSOData(IDeserializer& deserializer) {
        SR_UTILS_NS::SceneObject::Ptr pSO;

        auto&& pScene = GetScene();
        if (!pScene) {
            SRHalt("ScenePrefabLogic::LoadLogic() : scene is nullptr!");
            return false;
        }

        if (deserializer.BeginObject(SR_UTILS_NS::SerializationId::Create("info"))) {
            std::string type;
            deserializer.ReadString(type, SR_UTILS_NS::SerializationId::Create("type"));
            pSO = SR_UTILS_NS::Factory::Instance().Create<SceneObject>(type);
            deserializer.EndObject();
        }
        else {
            /// Empty prefab, create default SceneObject or load from custom data
            return true;
        }

        if (!pSO) {
            SRHalt("ScenePrefabLogic::LoadLogic() : failed to create scene object!");
            return false;
        }

        if (deserializer.BeginObject(SR_UTILS_NS::SerializationId::Create("data"))) {
            pSO->Load(deserializer);
            deserializer.EndObject();
        }
        else {
            pSO.AutoFree([](SceneObject* pData, SR_HTYPES_NS::SharedPtrDynamicData*) {
                pData->Destroy();
            });
            SRHalt("ScenePrefabLogic::LoadLogic() : failed to load data object!");
            return false;
        }

        pScene->RegisterSceneObject(pSO);

        return true;
    }

    bool ScenePrefabLogic::SaveLogic(ISerializer& serializer, const Path& path) {
        SR_TRACY_ZONE;
        SR_UTILS_NS::SceneObject::Ptr pSO;
        uint64_t rootObjectsCount = 0;

        if (auto&& pScene = GetScene(); SRVerify2(pScene, "Scene is nullptr!")) {
            for (auto&& pObject : pScene->GetRootSceneObjects()) {
                if (pObject && !pObject->HasSerializationFlags(SerializationFlags::DontSave)) {
                    ++rootObjectsCount;
                    pSO = pObject;
                }
            }
            if (rootObjectsCount == 0) {
                /// Empty prefab, create default SceneObject or load from custom data
            }
            else if (rootObjectsCount != 1) {
                SRHalt("ScenePrefabLogic::SaveLogic() : invalid root objects count!");
                return false;
            }
        }

        if (rootObjectsCount == 1 && !SaveSOAsPrefab(serializer, pSO)) {
            SR_ERROR("ScenePrefabLogic::SaveLogic() : failed to save scene object as prefab!");
            return false;
        }

        return Super::SaveLogic(serializer, path);
    }

    bool ScenePrefabLogic::LoadLogic(IDeserializer& deserializer, const Path& path) {
        if (!LoadSOData(deserializer)) {
            SR_ERROR("ScenePrefabLogic::LoadLogic() : failed to load scene object data!");
            return false;
        }

        return Super::LoadLogic(deserializer, path);
    }

    void ScenePrefabLogic::InitLogic() {
        Super::InitLogic();

        if (m_pSOCustomData) {
            LoadSOData(*m_pSOCustomData);
            m_pSOCustomData = nullptr;
        }

        SceneObject::Ptr pSO = GetPrefabRoot();

        if (!pSO) {
            pSO = DynamicPointerCast<SceneObject>(GetScene()->InstanceGameObject("Root"));
        }

        pSO->AddEditorFlags(EditorFlags::DontDelete);
    }

    SR_HTYPES_NS::SharedPtr<SceneObject> ScenePrefabLogic::GetPrefabRoot() const noexcept {
        auto&& root = GetScene()->GetRootSceneObjects();
        for (auto&& pObject : root) {
            if (pObject && !pObject->HasSerializationFlags(SerializationFlags::DontSave)) {
                return pObject;
            }
        }

        return nullptr;
    }

    void ScenePrefabLogic::SetCustomSOData(IDeserializer::UniquePtr pCustomData) {
        m_pSOCustomData = std::move(pCustomData);
    }
}
