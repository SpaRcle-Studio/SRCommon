//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_UI_LAYOUT_REBUILDER_H
#define SR_ENGINE_UTILS_UI_LAYOUT_REBUILDER_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    class SceneObject;
    class ILayoutElement;
    class ILayoutController;

    namespace LayoutRebuilder {
        struct Node {
            uint64_t elementBegin = 0;
            uint64_t elementCount = 0;
            uint64_t selfBegin = 0;
            uint64_t selfCount = 0;
            uint64_t controllerBegin = 0;
            uint64_t controllerCount = 0;
        };

        struct Context {
            Vector<Node> nodes;
            Vector<ILayoutElement*> elements;
            Vector<ILayoutController*> selfControllers;
            Vector<ILayoutController*> controllers;
        };

        SR_COMMON_DLL_API void MarkDirty(SceneObject& object, bool fromTransform = false);
        SR_COMMON_DLL_API void Rebuild(SceneObject& root);
        SR_COMMON_DLL_API void Rebuild(SceneObject& root, Context& context);
    }
}

#endif // SR_ENGINE_UTILS_UI_LAYOUT_REBUILDER_H
