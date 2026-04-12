//
// Created by Monika on 24.12.2022.
//

#ifndef SR_ENGINE_SCENE_PREFAB_LOGIC_H
#define SR_ENGINE_SCENE_PREFAB_LOGIC_H

#include <Utils/World/SceneLogic.h>

namespace SR_WORLD_NS {
    class ScenePrefabLogic : public SceneLogic {
        SR_CLASS()
        using Super = SceneLogic;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<ScenePrefabLogic>;

    public:
        ~ScenePrefabLogic() override;

        void InitLogic() override;
        bool SaveLogic(ISerializer& serializer, const Path& path) override;
        bool LoadLogic(IDeserializer& deserializer, const Path& path) override;

        SR_NODISCARD SceneLogicType GetType() const noexcept override { return SceneLogicType::Prefab; }
        SR_NODISCARD StringAtom GetSceneExtension() const noexcept override { return "prefab"; }
        SR_NODISCARD bool IsAllowedRootSave() const noexcept override { return false; }
        SR_NODISCARD SR_HTYPES_NS::SharedPtr<SceneObject> GetPrefabRoot() const noexcept;

        static bool SaveSOAsPrefab(ISerializer& serializer, const SR_HTYPES_NS::SharedPtr<SceneObject>& pSO);

        void SetCustomSOData(std::unique_ptr<IDeserializer> pCustomData);

    private:
        bool LoadSOData(IDeserializer& deserializer);

    private:
        std::unique_ptr<IDeserializer> m_pSOCustomData;

    };
}

#endif //SR_ENGINE_SCENE_PREFAB_LOGIC_H
