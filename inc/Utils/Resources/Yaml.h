//
// Created by innerviewer on 2024-06-08.
//

#ifndef SR_COMMON_YAML_H
#define SR_COMMON_YAML_H

#include <rapidyaml/src/ryml.hpp>

namespace SR_UTILS_NS::Yaml {
    class SR_DLL_EXPORT Node {
        friend class Document;

    public:
        Node();
        explicit Node(ryml::ConstNodeRef node);

    public:
        static Node Empty() { return { }; }

        explicit operator bool() const { return IsValid(); }
        SR_NODISCARD bool IsValid() const { return m_isValid; }

        SR_NODISCARD std::string Name() const;
        SR_NODISCARD std::string_view NameView() const;

        SR_NODISCARD std::string GetValue() const;
        SR_NODISCARD std::string GetKey() const;
        SR_NODISCARD std::vector<Node> GetChildren() const;
        SR_NODISCARD uint16_t GetId() const { return m_node.m_id; }
        SR_NODISCARD Node GetChild(const std::string& name) const;

    private:
        ryml::ConstNodeRef m_node;
        bool m_isValid = false;
    };

    class SR_DLL_EXPORT Document final : public NonCopyable {
    public:
        Document();

        Document(Document&& document) noexcept;
        Document& operator=(Document&& document) noexcept;
        explicit operator bool() const { return IsValid(); }

    public:
        static Document Empty() { return { }; }

        static Document New();
        static Document Load(const SR_UTILS_NS::Path &path);

    public:
        Node AppendChild(const std::string& name);

        SR_NODISCARD bool Save(const SR_UTILS_NS::Path& path) const;
        SR_NODISCARD std::string Dump() const;

        SR_NODISCARD Node GetRoot() const;
        SR_NODISCARD bool IsValid() const { return m_isValid; }

    private:
        ryml::Tree m_tree;
        SR_UTILS_NS::Path m_path;

        bool m_isValid;
    };
}

#endif //SR_COMMON_YAML_H
