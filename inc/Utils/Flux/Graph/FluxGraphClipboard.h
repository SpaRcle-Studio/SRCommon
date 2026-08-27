//
// Created by Monika on 26.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_GRAPH_CLIPBOARD_H
#define SR_ENGINE_COMMON_FLUX_GRAPH_CLIPBOARD_H

#include <Utils/Flux/Graph/FluxGraph.h>

namespace SR_FLUX_NS {
    /// Копирование узлов графа Flux через системный буфер обмена.
    /// Позиции узлов сохраняются относительно центра выделения, поэтому вставка не зависит
    /// от того, где узлы находились в исходном графе - центр группы попадает в точку вставки
    class FluxGraphClipboard {
    public:
        /// Сохраняет выделенные узлы в буфер обмена. Связи сохраняются только между узлами
        /// из выделения, ссылки на внешние узлы отбрасываются
        /// @return false, если сохранять нечего
        static bool Copy(const FluxGraph& graph, const Vector<const FluxGraphNode*>& nodes);

        /// Вставляет содержимое буфера обмена в граф
        /// @param center точка в координатах канваса, в которую попадёт центр вставленных узлов
        /// @return индексы добавленных узлов
        SR_NODISCARD static Vector<uint32_t> Paste(FluxGraph& graph, const SR_MATH_NS::FVector2& center);

        /// Содержит ли буфер обмена данные графа Flux
        SR_NODISCARD static bool HasData();

    };
}

#endif //SR_ENGINE_COMMON_FLUX_GRAPH_CLIPBOARD_H
