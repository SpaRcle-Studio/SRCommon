//
// Created by Monika on 24.12.2022.
//

#include <Utils/World/ScenePrefabLogic.h>
#include <Utils/ECS/Transform3D.h>
#include <Utils/ECS/LayerManager.h>

namespace SR_WORLD_NS {
    /*bool ScenePrefabLogic::Save(const Path& path) {
        auto&& rootObjects = m_scene->GetRootSceneObjects();
        if (rootObjects.size() != 1) {
            SRHalt("ScenePrefabLogic::Save() : invalid root objects count!");
            return false;
        }

        auto&& pRootSO = rootObjects.front();
        if (!pRootSO) {
            SRHalt("ScenePrefabLogic::Save() : invalid root object!");
            return false;
        }

        SR_UTILS_NS::SRASerializer serializer;
        pRootSO->Save(serializer);

        if (!serializer.SaveToFile(path)) {
            SR_ERROR("ScenePrefabLogic::Save() : failed to save prefab!\n\tPath: " + path.ToString());
            return true;
        }

        return false;
    }

    bool ScenePrefabLogic::Load(const Path& path) {
        auto&& pPrefab = Prefab::Load(path);
        if (!pPrefab) {
            SR_ERROR("ScenePrefabLogic::Load() : failed to load prefab!\n\tPath: " + path.ToString());
            return false;
        }

        pPrefab->AddUsePoint();

        if (const SR_UTILS_NS::SceneObject::Ptr& pSO = pPrefab->GetData()) {
            m_scene->RegisterSceneObject(pSO);
        }

        pPrefab->RemoveUsePoint();

        return true;
    }*/

    void ScenePrefabLogic::OnPreSave() {
        if (auto&& pScene = GetScene(); SRVerify2(pScene, "Scene is nullptr!")) {
            if (pScene->GetRootSceneObjects().size() != 1) {
                SRHalt("ScenePrefabLogic::OnPreSave() : invalid root objects count!");
            }
        }

        Super::OnPreSave();
    }
}
