//
// Created by Monika on 15.08.2024.
//

#ifndef SR_COMMON_WEB_HTML_H
#define SR_COMMON_WEB_HTML_H

#include <Utils/Web/CSS/CSS.h>
#include <Utils/Web/HTML/Tag.h>
#include <Utils/Types/SharedPtr.h>

namespace SR_UTILS_NS::Web {
    class HTMLAttribute : public SR_UTILS_NS::NonCopyable {
    public:
        void SetName(std::string&& name) { m_name = std::move(name); }
        void SetValue(std::string&& value) { m_value = std::move(value); }

        SR_NODISCARD const std::string& GetName() const { return m_name; }
        SR_NODISCARD const std::string& GetValue() const { return m_value; }

    private:
        std::string m_name;
        std::string m_value;

    };

    class HTMLNode : public SR_UTILS_NS::NonCopyable {
    public:
        ~HTMLNode() override;

    public:
        void SetParent(HTMLNode* pParent) { m_pParent = pParent; }
        void SetTag(const HTMLTag tag) { m_tag = tag; }
        void SetNodeName(std::string&& nodeName) { m_nodeName = std::move(nodeName); }
        void SetText(std::string&& text) { m_text = std::move(text); }

        void AddChild(HTMLNode* pChild) { m_children.emplace_back(pChild); }
        void AddAttribute(HTMLAttribute* pAttribute) { m_attributes.emplace_back(pAttribute); }

        SR_NODISCARD HTMLNode* GetParent() const { return m_pParent; }
        SR_NODISCARD HTMLTag GetTag() const { return m_tag; }
        SR_NODISCARD const std::string& GetNodeName() const;
        SR_NODISCARD const std::string& GetText() const { return m_text; }

        SR_NODISCARD const std::vector<HTMLNode*>& GetChildren() const { return m_children; }
        SR_NODISCARD const std::vector<HTMLAttribute*>& GetAttributes() const { return m_attributes; }

    private:
        HTMLNode* m_pParent = nullptr;
        HTMLTag m_tag = HTMLTag::Undefined;
        //CSSStyle m_style;
        std::string m_nodeName;
        std::string m_text;
        std::vector<HTMLNode*> m_children;
        std::vector<HTMLAttribute*> m_attributes;

    };

    class HTMLPage final : public SR_HTYPES_NS::SharedPtr<HTMLPage> {
        using Super = SR_HTYPES_NS::SharedPtr<HTMLPage>;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<HTMLPage>;

    public:
        HTMLPage();
        ~HTMLPage();

    public:
        SR_NODISCARD HTMLNode* GetHead() const { return m_pHead; }
        SR_NODISCARD HTMLNode* GetBody() const { return m_pBody; }
        SR_NODISCARD const SR_MATH_NS::UVector2& GetSize() const { return m_size; }

        void SetSize(const SR_MATH_NS::UVector2& size) { m_size = size; }
        void SetHead(HTMLNode* pHead) { m_pHead = pHead; }
        void SetBody(HTMLNode* pBody) { m_pBody = pBody; }

    private:
        HTMLNode* m_pHead = nullptr;
        HTMLNode* m_pBody = nullptr;
        SR_MATH_NS::UVector2 m_size;
    };
}

#endif //SR_COMMON_WEB_HTML_H
