//
// Created by Monika on 24.12.2022.
//

#include <Utils/World/ScenePrefabLogic.h>
#include <Utils/ECS/Transform3D.h>
#include <Utils/ECS/LayerManager.h>

#include <Codegen/ScenePrefabLogic.generated.hpp>

namespace SR_WORLD_NS {
    bool ScenePrefabLogic::SaveLogic(ISerializer& serializer, const Path& path) {
        SR_UTILS_NS::SceneObject::Ptr pSO;

        if (auto&& pScene = GetScene(); SRVerify2(pScene, "Scene is nullptr!")) {
            uint64_t rootObjectsCount = 0;
            for (auto&& pObject : pScene->GetRootSceneObjects()) {
                if (pObject && !pObject->HasSerializationFlags(SerializationFlags::DontSave)) {
                    ++rootObjectsCount;
                    pSO = pObject;
                }
            }
            if (rootObjectsCount != 1) {
                SRHalt("ScenePrefabLogic::SaveLogic() : invalid root objects count!");
                return false;
            }
        }

        SRAssert(pSO);

        serializer.BeginObject(SR_UTILS_NS::SerializationId::Create("info"));
        {
            serializer.WriteString(SR_UTILS_NS::EnumReflector::ToStringAtom(pSO->GetSceneObjectType()), SR_UTILS_NS::SerializationId::Create("type"));
        }
        serializer.EndObject();

        serializer.BeginObject(SR_UTILS_NS::SerializationId::Create("data"));
        {
            pSO->Save(serializer);
        }
        serializer.EndObject();

        return Super::SaveLogic(serializer, path);
    }

    bool ScenePrefabLogic::LoadLogic(IDeserializer& deserializer, const Path& path) {
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
            SRHalt("ScenePrefabLogic::LoadLogic() : failed to load info object!");
            return false;
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
            pSO.AutoFree([](auto&& pData) {
                pData->Destroy();
            });
            SRHalt("ScenePrefabLogic::LoadLogic() : failed to load data object!");
            return false;
        }

        pScene->RegisterSceneObject(pSO);

        return Super::LoadLogic(deserializer, path);
    }

    void ScenePrefabLogic::InitLogic() {
        Super::InitLogic();

        SceneObject::Ptr pSO = GetPrefabRoot();

        if (!pSO) {
            pSO = GetScene()->InstanceGameObject("Root").StaticCast<SceneObject>();
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
}
