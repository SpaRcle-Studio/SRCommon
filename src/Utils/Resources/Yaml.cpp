//
// Created by innerviewer on 2024-06-08.
//

#include <Utils/Resources/Yaml.h>
#include <Utils/FileSystem/Path.h>

namespace SR_UTILS_NS::Yaml {
    Node::Node()
        : m_node()
        , m_isValid(false)
    { }

    Node::Node(ryml::ConstNodeRef node) {
        m_node = node;
        m_isValid = m_node.valid();
    }

    std::string Node::Name() const {
        if (!m_isValid) {
            SRHalt("Node::Name() : node is not valid!");
            return { };
        }

        return m_node.key().str;
    }

    std::string Node::GetValue() const {
        if (!m_isValid) {
            SRHalt("Node::GetAttribute() : node is not valid!");
            return { };
        }

        return { m_node.val().begin(), m_node.val().end() };
    }

    std::string Node::GetKey() const {
        if (!m_isValid) {
            SRHalt("Node::GetKey() : node is not valid!");
            return { };
        }

        return { m_node.key().begin(), m_node.key().end() };
    }

    std::vector<Node> Node::GetChildren() const {
        std::vector<Node> children;
        children.resize(m_node.num_children());

        for (auto&& child : m_node.children()) {
            children.emplace_back(child);
        }

        return children;
    }

    SR_NODISCARD Node Node::GetChild(const std::string& name) const {
        if (!m_isValid) {
            return { };
        }

        if (!m_node.has_child(name.c_str())) {
            return { };
        }

        auto&& child = m_node.find_child(name.c_str());
        if (!child.valid()) {
            return { };
        }

        return Node(child);
    }

    std::string_view Node::NameView() const {
        if (!m_isValid) {
            SRHalt("Node::Name() : node is not valid!");
            return {};
        }

        return m_node.key().str;
    }

    Document::Document() {
        m_isValid = false;
    }

    Document::Document(Document&& document) noexcept
        : m_tree(std::exchange(document.m_tree, {}))
        , m_isValid(std::exchange(document.m_isValid, {}))
        , m_path(std::exchange(document.m_path, {}))
    { }

    Document& Document::operator=(Document&& document) noexcept {
        m_tree = std::exchange(document.m_tree, {});
        m_isValid = std::exchange(document.m_isValid, {});
        m_path = std::exchange(document.m_path, {});
        return *this;
    }

    Document Document::New() {
        Document yaml = Document();
        yaml.m_isValid = true;
        return yaml;
    }

    Node Document::AppendChild(const std::string& name) {
        if (!m_isValid) {
            SRHalt("Document::AppendChild() : document is not valid!");
            return { };
        }

        auto&& nodeId = m_tree.append_child(m_tree.root_id());
        m_tree.get(nodeId)->m_key.scalar.assign(name.c_str());
        return Node(m_tree.cref(nodeId));
    }

    Document Document::Load(const Path &path) {
        SR_TRACY_ZONE;
        SR_TRACY_TEXT_N("Path", path.ToStringRef());

        auto&& contents = FileSystem::ReadAllText(path.ToStringRef());
        Document yaml = Document::New();

        if (contents.empty()) {
            SR_ERROR("Document::Load() : failed to read YAML contents! \n\tPath: " + path.ToString());
            return { };
        }

        ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(contents));
        if (tree.empty()) {
            SR_ERROR("ThreadsWorker::Load() : failed to parse file \"{}\"", path.ToStringRef());
            yaml.m_isValid = false;
            return { };
        }

        yaml.m_isValid = true;
        yaml.m_tree = tree;
        yaml.m_path = std::move(path.ToString());

        return yaml;
    }

    bool Document::Save(const SR_UTILS_NS::Path& path) const {
        if (!path.Exists()) {
            if (!path.Create()) {
                SR_ERROR("Document::Save() : failed to create path!\n\tPath: '{}'", path.c_str());
                return false;
            }
        }

        std::ofstream file(path.ToStringRef());
        if (!file) {
            SR_ERROR("Document::Save() : failed to open file. \n\tPath: '{}'", path.c_str());
            return false;
        }

        for (auto&& node : m_tree.m_arena) {
            file << node;
        }

        file.close();
        return true;
    }

    SR_NODISCARD Node Document::GetRoot() const {
        if (!IsValid()) {
            return { };
        }

        ryml::ConstNodeRef root = m_tree.crootref();
        if (!root.valid() || !root.has_children()) {
            SR_ERROR("Document::TryRoot() : failed to get root node from file!");
            return { };
        }

        return Node(root);
    }
}