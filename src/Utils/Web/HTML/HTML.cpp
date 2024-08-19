//
// Created by Monika on 15.08.2024.
//

#include <Utils/Web/HTML/HTML.h>

namespace SR_UTILS_NS::Web {
    HTMLNode::~HTMLNode() {
        for (auto&& pChild : m_children) {
            SR_SAFE_DELETE_PTR(pChild);
        }
        m_children.clear();

        for (auto&& pAttribute : m_attributes) {
            SR_SAFE_DELETE_PTR(pAttribute);
        }
        m_attributes.clear();
    }

    const std::string& HTMLNode::GetNodeName() const {
        if (m_nodeName.empty()) {
            const SR_UTILS_NS::StringAtom tagName = HTMLTagToStringAtom(m_tag);
            return tagName.ToStringRef();
        }
        return m_nodeName;
    }

    HTMLPage::HTMLPage()
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
    { }

    HTMLPage::~HTMLPage() {
        SR_SAFE_DELETE_PTR(m_pHead);
        SR_SAFE_DELETE_PTR(m_pBody);
    }
}
