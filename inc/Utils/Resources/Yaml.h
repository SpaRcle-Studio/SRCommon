//
// Created by innerviewer on 2024-06-08.
//

#ifndef SR_COMMON_YAML_H
#define SR_COMMON_YAML_H

#include <Utils/Common/NonCopyable.h>

namespace SR_UTILS_NS::Yaml {
    class SR_COMMON_DLL_API Node {
        friend class Document;

    public:
        Node() = default;

        explicit Node(void const* pTreeImpl, const size_t id)
            : m_constTreeImpl(pTreeImpl)
            , m_id(id)
        { }

        explicit Node(void* pTreeImpl, const size_t id)
            : m_treeImpl(pTreeImpl)
            , m_id(id)
        { }

    public:
        static Node Empty() { return Node(); }

        explicit operator bool() const { return IsValid(); }
        SR_NODISCARD bool IsValid() const { return (m_treeImpl || m_constTreeImpl) && m_id != -1; }

        SR_NODISCARD std::string Name() const;
        SR_NODISCARD std::string_view NameView() const;

        SR_NODISCARD void SetValue(const std::string_view& value);
        SR_NODISCARD std::string GetValue() const;
        SR_NODISCARD std::string_view GetValueView() const;
        SR_NODISCARD std::string GetKey() const;
        SR_NODISCARD std::string_view GetKeyView() const;
        SR_NODISCARD std::vector<Node> GetChildren() const;
        SR_NODISCARD uint16_t GetId() const;
        SR_NODISCARD Node GetChild(const std::string_view& name) const;
        SR_NODISCARD Node AppendChild(const std::string_view& name);

    private:
        void* GetTreeImpl() { return m_treeImpl; }
        void const* GetTreeImpl() const { return m_constTreeImpl ? m_constTreeImpl : m_treeImpl; }

    private:
        void const* m_constTreeImpl = nullptr;
        void* m_treeImpl = nullptr;
        size_t m_id = -1;
    };

    class SR_COMMON_DLL_API Document final : public NonCopyable {
    public:
        Document() = default;
        ~Document() override;

        Document(Document&& document) noexcept;
        Document& operator=(Document&& document) noexcept;
        explicit operator bool() const { return IsValid(); }

    public:
        static Document Empty();
        static Document New();
        static Document Load(const SR_UTILS_NS::Path &path);

    public:
        Node AppendChild(const std::string_view& name);

        SR_NODISCARD bool Save(const SR_UTILS_NS::Path& path) const;
        SR_NODISCARD std::string Dump() const;

        SR_NODISCARD Node GetRoot() const;
        SR_NODISCARD Node GetRoot();
        SR_NODISCARD bool IsValid() const { return m_pImpl; }

    private:
        void* m_pImpl = nullptr;
        SR_UTILS_NS::Path m_path;
    };
}

#endif //SR_COMMON_YAML_H
