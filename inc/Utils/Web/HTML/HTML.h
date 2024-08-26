//
// Created by Monika on 15.08.2024.
//

#ifndef SR_COMMON_WEB_HTML_H
#define SR_COMMON_WEB_HTML_H

#include <Utils/Types/ObjectPool.h>
#include <Utils/Web/CSS/CSS.h>
#include <Utils/Web/HTML/Tag.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Common/PassKey.h>

namespace SR_UTILS_NS::Web {
    class HTMLPage;
    class HTMLNode;

    class HTMLAttribute : public SR_UTILS_NS::NonCopyable {
    public:
        HTMLAttribute(HTMLPage* pPage, Passkey<HTMLPage>)
            : m_pPage(pPage)
        { }

        HTMLAttribute(HTMLAttribute&& other) noexcept
            : m_pPage(SR_EXCHANGE(other.m_pPage, nullptr))
            , m_pNode(SR_EXCHANGE(other.m_pNode, nullptr))
            , m_name(SR_EXCHANGE(other.m_name, { }))
            , m_value(SR_EXCHANGE(other.m_value, { }))
            , m_id(SR_EXCHANGE(other.m_id, SR_ID_INVALID))
        { }

        ~HTMLAttribute() override;

        HTMLAttribute& operator=(HTMLAttribute&& other) noexcept {
            m_pPage = SR_EXCHANGE(other.m_pPage, nullptr);
            m_pNode = SR_EXCHANGE(other.m_pNode, nullptr);
            m_name = SR_EXCHANGE(other.m_name, { });
            m_value = SR_EXCHANGE(other.m_value, { });
            m_id = SR_EXCHANGE(other.m_id, SR_ID_INVALID);
            return *this;
        }

    public:
        void SetName(std::string&& name) { m_name = std::move(name); }
        void SetValue(std::string&& value) { m_value = std::move(value); }

        void SetNode(HTMLNode* pNode, Passkey<HTMLNode>) { m_pNode = pNode; }
        void SetId(const uint64_t id, Passkey<HTMLPage>) { m_id = id; }

        SR_NODISCARD const std::string& GetName() const { return m_name; }
        SR_NODISCARD const std::string& GetValue() const { return m_value; }

        SR_NODISCARD HTMLNode* GetNode() const { return m_pNode; }
        SR_NODISCARD uint64_t GetId() const { return m_id; }

    private:
        std::string m_name;
        std::string m_value;

        HTMLPage* m_pPage = nullptr;
        HTMLNode* m_pNode = nullptr;

        uint64_t m_id = SR_ID_INVALID;

    };

    class HTMLNode : public SR_UTILS_NS::NonCopyable {
        using Super = SR_UTILS_NS::NonCopyable;
    public:
        HTMLNode(HTMLPage* pPage, Passkey<HTMLPage>)
            : Super()
            , m_pPage(pPage)
        { }

        HTMLNode(HTMLNode&& other) noexcept
            : m_pPage(SR_EXCHANGE(other.m_pPage, { }))
            , m_parentId(SR_EXCHANGE(other.m_parentId, { }))
            , m_tag(SR_EXCHANGE(other.m_tag, HTMLTag::Undefined))
            , m_style(SR_EXCHANGE(other.m_style, { }))
            , m_nodeName(SR_EXCHANGE(other.m_nodeName, { }))
            , m_text(SR_EXCHANGE(other.m_text, { }))
            , m_children(SR_EXCHANGE(other.m_children, { }))
            , m_attributes(SR_EXCHANGE(other.m_attributes, { }))
            , m_id(SR_EXCHANGE(other.m_id, SR_ID_INVALID))
        { }

        HTMLNode& operator=(HTMLNode&& other) noexcept {
            m_pPage = SR_EXCHANGE(other.m_pPage, { });
            m_parentId = SR_EXCHANGE(other.m_parentId, { });
            m_tag = SR_EXCHANGE(other.m_tag, HTMLTag::Undefined);
            m_style = SR_EXCHANGE(other.m_style, { });
            m_nodeName = SR_EXCHANGE(other.m_nodeName, { });
            m_text = SR_EXCHANGE(other.m_text, { });
            m_children = SR_EXCHANGE(other.m_children, { });
            m_attributes = SR_EXCHANGE(other.m_attributes, { });
            m_id = SR_EXCHANGE(other.m_id, SR_ID_INVALID);
            return *this;
        }

        ~HTMLNode() override;

    public:
        void SetParent(const HTMLNode* pParent) { m_parentId = pParent->GetId(); }
        void SetParent(const uint64_t parentId) { m_parentId = parentId; }
        void SetTag(const HTMLTag tag) { m_tag = tag; }
        void SetNodeName(std::string&& nodeName) { m_nodeName = std::move(nodeName); }
        void SetText(std::string&& text) { m_text = std::move(text); }

        void AddChild(const HTMLNode* pChild) { m_children.emplace_back(pChild->GetId()); }
        void AddAttribute(const HTMLAttribute* pAttribute) { m_attributes.emplace_back(pAttribute->GetId()); }

        void SetStyle(const CSSStyle& style) { m_style = style; }

        SR_NODISCARD HTMLNode* GetParent() const;
        SR_NODISCARD HTMLTag GetTag() const { return m_tag; }
        SR_NODISCARD const std::string& GetNodeName() const;
        SR_NODISCARD const std::string& GetText() const { return m_text; }

        SR_NODISCARD CSSStyle& GetStyle() { return m_style; }
        SR_NODISCARD const CSSStyle& GetStyle() const { return m_style; }

        SR_NODISCARD const std::vector<uint64_t>& GetChildren() const { return m_children; }
        SR_NODISCARD const std::vector<uint64_t>& GetAttributes() const { return m_attributes; }

        SR_NODISCARD HTMLAttribute* GetAttributeByName(const std::string& name) const;

        SR_NODISCARD HTMLPage* GetPage() const { return m_pPage; }

        SR_NODISCARD uint64_t GetId() const { return m_id; }
        void SetId(const uint64_t id, Passkey<HTMLPage>) { m_id = id; }

        void SetUserData(void* pUserData) { m_pUserData = pUserData; }
        SR_NODISCARD void* GetUserData() const { return m_pUserData; }

        void RemoveUserDataRecursively();

    private:
        HTMLPage* m_pPage = nullptr;
        uint64_t m_parentId = SR_ID_INVALID;
        HTMLTag m_tag = HTMLTag::Undefined;
        CSSStyle m_style;
        std::string m_nodeName;
        std::string m_text;
        std::vector<uint64_t> m_children;
        std::vector<uint64_t> m_attributes;

        uint64_t m_id = SR_ID_INVALID;

        void* m_pUserData = nullptr;

    };

    class HTMLPage final : public SR_HTYPES_NS::SharedPtr<HTMLPage> {
        using Super = SR_HTYPES_NS::SharedPtr<HTMLPage>;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<HTMLPage>;

    public:
        HTMLPage();
        ~HTMLPage();

        SR_NODISCARD HTMLNode* AllocateNode();
        void FreeNode(HTMLNode* pNode);
        void FreeNode(const uint64_t id) { FreeNode(GetNodeById(id)); }

        SR_NODISCARD HTMLAttribute* AllocateAttribute();
        void FreeAttribute(HTMLAttribute* pAttribute);
        void FreeAttribute(const uint64_t id) { FreeAttribute(GetAttributeById(id)); }

    public:
        SR_NODISCARD HTMLNode* GetHead() { return m_headId != SR_ID_INVALID ? &m_nodePool.AtUnchecked(m_headId) : nullptr; }
        SR_NODISCARD HTMLNode* GetBody() { return m_bodyId != SR_ID_INVALID ? &m_nodePool.AtUnchecked(m_bodyId) : nullptr; }
        SR_NODISCARD const HTMLNode* GetHead() const { return m_headId != SR_ID_INVALID ? &m_nodePool.AtUnchecked(m_headId) : nullptr; }
        SR_NODISCARD const HTMLNode* GetBody() const { return m_bodyId != SR_ID_INVALID ? &m_nodePool.AtUnchecked(m_bodyId) : nullptr; }

        SR_NODISCARD SR_MATH_NS::UVector2 GetSize() const;

        SR_NODISCARD HTMLNode* GetNodeById(uint64_t id);
        SR_NODISCARD HTMLAttribute* GetAttributeById(uint64_t id);

        void AddStyle(const CSS::Ptr& pStyle) { m_styles.emplace_back(pStyle); }

        SR_NODISCARD std::optional<CSSStyle> GetStyle(const std::string& selector) const {
            for (const auto& pStyle : m_styles) {
                if (const auto pStyleValue = pStyle->GetStyle(selector)) {
                    return *pStyleValue;
                }
            }
            return std::nullopt;
        }

        void SetSize(const SR_MATH_NS::UVector2& size);
        void SetHead(const HTMLNode* pHead) { m_headId = pHead->GetId(); }
        void SetBody(const HTMLNode* pBody) { m_bodyId = pBody->GetId(); }

        void RemoveUserDataRecursively();

    private:
        uint64_t m_headId = SR_ID_INVALID;
        uint64_t m_bodyId = SR_ID_INVALID;

        SR_HTYPES_NS::ObjectPool<HTMLNode> m_nodePool;
        SR_HTYPES_NS::ObjectPool<HTMLAttribute> m_attributePool;

        std::vector<CSS::Ptr> m_styles;

    };
}

#endif //SR_COMMON_WEB_HTML_H
