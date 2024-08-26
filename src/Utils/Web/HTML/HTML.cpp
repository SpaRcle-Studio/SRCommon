//
// Created by Monika on 15.08.2024.
//

#include <Utils/Profile/TracyContext.h>
#include <Utils/Web/HTML/HTML.h>

namespace SR_UTILS_NS::Web {
    HTMLAttribute::~HTMLAttribute() {
        SRAssert2(m_id == SR_ID_INVALID, "Incorrect attribute destruction!");
    }

    HTMLNode::~HTMLNode() {
        SRAssert2(m_id == SR_ID_INVALID, "Incorrect node destruction!");

        for (auto&& pChild : m_children) {
            m_pPage->FreeNode(pChild);
        }
        m_children.clear();

        for (auto&& pAttribute : m_attributes) {
            m_pPage->FreeAttribute(pAttribute);
        }
        m_attributes.clear();
    }

    HTMLNode* HTMLNode::GetParent() const {
        return m_parentId != SR_ID_INVALID ? m_pPage->GetNodeById(m_parentId) : nullptr;
    }

    const std::string& HTMLNode::GetNodeName() const {
        if (m_nodeName.empty()) {
            const SR_UTILS_NS::StringAtom tagName = HTMLTagToStringAtom(m_tag);
            return tagName.ToStringRef();
        }
        return m_nodeName;
    }

    HTMLAttribute* HTMLNode::GetAttributeByName(const std::string &name) const {
        for (const auto& attributeId : m_attributes) {
            auto&& pAttribute = m_pPage->GetAttributeById(attributeId);
            if (pAttribute->GetName() == name) {
                return pAttribute;
            }
        }
        return nullptr;
    }

    HTMLPage::HTMLPage()
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
    { }

    HTMLPage::~HTMLPage() {
        FreeNode(m_headId);
        FreeNode(m_bodyId);
    }

    HTMLNode* HTMLPage::AllocateNode() {
        const uint64_t index = m_nodePool.Add(HTMLNode(this, Passkey(this)));
        auto&& pNode = &m_nodePool.AtUnchecked(index);
        pNode->SetId(index, Passkey(this));
        return pNode;
    }

    void HTMLPage::FreeNode(HTMLNode* pNode) {
        if (SRVerify(pNode)) {
            /// pNode is already valid here
            const uint64_t index = pNode->GetId(); /// NOLINT
            pNode->SetId(SR_ID_INVALID, Passkey(this));
            m_nodePool.RemoveByIndex(index);
        }
    }

    HTMLAttribute* HTMLPage::AllocateAttribute() {
        const uint64_t index = m_attributePool.Add(HTMLAttribute(this, Passkey(this)));
        auto&& pAttribute = &m_attributePool.AtUnchecked(index);
        pAttribute->SetId(index, Passkey(this));
        return pAttribute;
    }

    void HTMLPage::FreeAttribute(HTMLAttribute* pAttribute) {
        if (SRVerify(pAttribute)) {
            /// pAttribute is already valid here
            const uint64_t index = pAttribute->GetId(); /// NOLINT
            pAttribute->SetId(SR_ID_INVALID, Passkey(this));
            m_attributePool.RemoveByIndex(index);
        }
    }

    SR_MATH_NS::UVector2 HTMLPage::GetSize() const {
        if (auto&& pBody = GetBody()) {
            return SR_MATH_NS::UVector2(pBody->GetStyle().width.GetPx(), pBody->GetStyle().height.GetPx());
        }
        return SR_MATH_NS::UVector2(0, 0);
    }

    HTMLNode* HTMLPage::GetNodeById(const uint64_t id) {
        return &m_nodePool.AtUnchecked(id);
    }

    HTMLAttribute* HTMLPage::GetAttributeById(const uint64_t id) {
        return &m_attributePool.AtUnchecked(id);
    }

    void HTMLPage::SetSize(const SR_MATH_NS::UVector2& size) {
        if (auto&& pBody = GetBody()) {
            pBody->GetStyle().width = CSSSizeValue(static_cast<float>(size.x), CSSSizeValue::Unit::Px);
            pBody->GetStyle().height = CSSSizeValue(static_cast<float>(size.y), CSSSizeValue::Unit::Px);
        }
    }

    void HTMLPage::RemoveUserDataRecursively() {
        SR_TRACY_ZONE;

        if (m_headId != SR_ID_INVALID) {
            GetNodeById(m_headId)->RemoveUserDataRecursively();
        }
        if (m_bodyId != SR_ID_INVALID) {
            GetNodeById(m_bodyId)->RemoveUserDataRecursively();
        }
    }

    void HTMLNode::RemoveUserDataRecursively() { /// NOLINT
        SR_TRACY_ZONE;

        m_pUserData = nullptr;

        for (auto&& pChild : m_children) {
            m_pPage->GetNodeById(pChild)->RemoveUserDataRecursively();
        }
    }
}
