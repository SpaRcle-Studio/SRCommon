//
// Created by Monika on 26.08.2026.
//

#include <Utils/Flux/Graph/FluxGraphClipboard.h>
#include <Utils/Debug.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Serialization/Serialization.h>
#include <Utils/Serialization/SerializableDataAccessor.h>
#include <Utils/Serialization/SRASerialization.h>

namespace SR_FLUX_NS {
    namespace {
        constexpr auto SR_FLUX_CLIPBOARD_ID = SerializationId::Create("SREngineFluxClipboard");
        constexpr auto SR_FLUX_CLIPBOARD_NODES_ID = SerializationId::Create("Nodes");
        constexpr auto SR_FLUX_CLIPBOARD_LINKS_ID = SerializationId::Create("Links");

        /// Индекс узла внутри выделения. Связи в буфере обмена адресуются локальными индексами,
        /// так как индексы в исходном графе к моменту вставки могут уже не существовать
        SR_NODISCARD uint32_t ToLocalIndex(const Vector<uint32_t>& selection, const uint32_t nodeIndex) {
            for (uint32_t localIndex = 0; localIndex < selection.size(); ++localIndex) {
                if (selection[localIndex] == nodeIndex) {
                    return localIndex;
                }
            }
            return SR_UINT32_MAX;
        }
    }

    bool FluxGraphClipboard::Copy(const FluxGraph& graph, const Vector<const FluxGraphNode*>& nodes) {
        if (nodes.empty()) {
            return false;
        }

        Vector<uint32_t> selection;
        selection.reserve(nodes.size());

        for (auto&& pNode : nodes) {
            const uint32_t nodeIndex = graph.GetNodeIndex(pNode);
            if (nodeIndex == SR_UINT32_MAX || selection.find(nodeIndex) != selection.end()) {
                continue;
            }
            selection.emplace_back(nodeIndex);
        }

        if (selection.empty()) {
            return false;
        }

        /// центр выделения по ограничивающему прямоугольнику: позиции сохраняются относительно него,
        /// чтобы вставка попадала под курсор независимо от исходного расположения узлов
        auto&& firstPosition = graph.GetNode(selection.front())->GetPosition();
        SR_MATH_NS::FVector2 min = firstPosition;
        SR_MATH_NS::FVector2 max = firstPosition;

        for (auto&& nodeIndex : selection) {
            auto&& position = graph.GetNode(nodeIndex)->GetPosition();
            min.x = SR_MIN(min.x, position.x);
            min.y = SR_MIN(min.y, position.y);
            max.x = SR_MAX(max.x, position.x);
            max.y = SR_MAX(max.y, position.y);
        }

        const SR_MATH_NS::FVector2 center = (min + max) * 0.5f;

        Vector<FluxGraphNode> serializedNodes;
        serializedNodes.reserve(selection.size());

        for (auto&& nodeIndex : selection) {
            auto&& node = serializedNodes.emplace_back(*graph.GetNode(nodeIndex));
            node.SetPosition(node.GetPosition() - center);
            node.SetUserData(nullptr);
        }

        Vector<FluxGraphLink> serializedLinks;

        for (auto&& link : graph.GetLinks()) {
            /// связи с узлами вне выделения теряют один из концов, поэтому не копируются
            const uint32_t sourceNode = ToLocalIndex(selection, link.GetSourceNode());
            const uint32_t targetNode = ToLocalIndex(selection, link.GetTargetNode());
            if (sourceNode == SR_UINT32_MAX || targetNode == SR_UINT32_MAX) {
                continue;
            }

            auto&& serializedLink = serializedLinks.emplace_back(link);
            serializedLink.SetSourceNode(sourceNode);
            serializedLink.SetTargetNode(targetNode);
            serializedLink.SetUserData(nullptr);
        }

        SRASerializer serializer;
        Serialization::Save(serializer, serializedNodes, SR_FLUX_CLIPBOARD_NODES_ID);
        Serialization::Save(serializer, serializedLinks, SR_FLUX_CLIPBOARD_LINKS_ID);

        String encodedData;
        StringUtils::Instance().Base64Encode(serializer.ToString(), encodedData);
        SR_PLATFORM_NS::TextToClipboard(SR_FLUX_CLIPBOARD_ID.GetName() + encodedData);

        return true;
    }

    Vector<uint32_t> FluxGraphClipboard::Paste(FluxGraph& graph, const SR_MATH_NS::FVector2& center) {
        Vector<uint32_t> pastedNodes;

        auto&& clipboard = SR_PLATFORM_NS::GetClipboardText();
        if (!clipboard.starts_with(SR_FLUX_CLIPBOARD_ID.GetName())) {
            return pastedNodes;
        }

        clipboard.erase(0, SR_FLUX_CLIPBOARD_ID.GetNameView().size());

        String decodedData;
        StringUtils::Instance().Base64Decode(clipboard, decodedData);

        SRADeserializer deserializer;
        if (!deserializer.LoadFromString(decodedData)) {
            return pastedNodes;
        }

        Vector<FluxGraphNode> nodes;
        Vector<FluxGraphLink> links;
        Serialization::Load(deserializer, nodes, SR_FLUX_CLIPBOARD_NODES_ID);
        Serialization::Load(deserializer, links, SR_FLUX_CLIPBOARD_LINKS_ID);

        if (nodes.empty()) {
            return pastedNodes;
        }

        pastedNodes.reserve(nodes.size());

        for (auto&& node : nodes) {
            node.SetPosition(node.GetPosition() + center);
            node.SetUserData(nullptr);
            pastedNodes.emplace_back(graph.AddNode(node));
        }

        for (auto&& link : links) {
            if (link.GetSourceNode() >= pastedNodes.size() || link.GetTargetNode() >= pastedNodes.size()) {
                SR_WARN("FluxGraphClipboard::Paste() : link references an unknown node!");
                continue;
            }

            FluxGraphLink pastedLink = link;
            pastedLink.SetSourceNode(pastedNodes[link.GetSourceNode()]);
            pastedLink.SetTargetNode(pastedNodes[link.GetTargetNode()]);
            pastedLink.SetUserData(nullptr);
            graph.AddLink(pastedLink);
        }

        return pastedNodes;
    }

    bool FluxGraphClipboard::HasData() {
        return SR_PLATFORM_NS::GetClipboardText().starts_with(SR_FLUX_CLIPBOARD_ID.GetName());
    }
}
