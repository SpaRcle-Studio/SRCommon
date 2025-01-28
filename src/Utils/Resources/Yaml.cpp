//
// Created by innerviewer on 2024-06-08.
//

#include <Utils/Resources/Yaml.h>
#include <Utils/FileSystem/Path.h>

#include <rapidyaml/src/ryml.hpp>

namespace SR_UTILS_NS::Yaml {
    std::string Node::Name() const {
        if (!IsValid()) {
            SRHalt("Node::Name() : node is not valid!");
            return { };
        }

        return ryml::ConstNodeRef(static_cast<ryml::Tree const*>(m_treeImpl), m_id).key().str;
    }

    std::string Node::GetValue() const {
        if (!IsValid()) {
            SRHalt("Node::GetValue() : node is not valid!");
            return { };
        }

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(m_treeImpl), m_id);
        return { node.val().begin(), node.val().end() };
    }

    std::string_view Node::GetValueView() const {
        if (!IsValid()) {
            SRHalt("Node::GetValueView() : node is not valid!");
            return {};
        }

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(m_treeImpl), m_id);
        return std::string_view(node.val().begin(), node.val().size());
    }

    std::string Node::GetKey() const {
        if (!IsValid()) {
            SRHalt("Node::GetKey() : node is not valid!");
            return { };
        }

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(m_treeImpl), m_id);
        return { node.key().begin(), node.key().end() };
    }

    std::string_view Node::GetKeyView() const {
        if (!IsValid()) {
            SRHalt("Node::GetKeyView() : node is not valid!");
            return {};
        }

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(m_treeImpl), m_id);
        return std::string_view(node.key().begin(), node.key().size());
    }

    std::vector<Node> Node::GetChildren() const {
        if (!IsValid()) {
            SRHalt("Node::GetChildren() : node is not valid!");
            return { };
        }

        std::vector<Node> children;

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(m_treeImpl), m_id);
        children.reserve(node.num_children());

        for (auto&& child : node.children()) {
            children.emplace_back(Node(static_cast<void const*>(child.m_tree), child.m_id));
        }

        return children;
    }

    uint16_t Node::GetId() const {
        if (!IsValid()) {
            SRHalt("Node::GetId() : node is not valid!");
            return { };
        }

        return m_id;
    }

    SR_NODISCARD Node Node::GetChild(const std::string& name) const {
        if (!IsValid()) {
            return { };
        }

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(m_treeImpl), m_id);
        if (!node.has_child(name.c_str())) {
            return { };
        }

        auto&& child = node.find_child(name.c_str());
        if (!child.valid()) {
            return { };
        }

        return Node(static_cast<void const*>(child.m_tree), child.m_id);
    }

    std::string_view Node::NameView() const {
        if (!IsValid()) {
            SRHalt("Node::NameView() : node is not valid!");
            return {};
        }

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(m_treeImpl), m_id);
        return node.key().str;
    }

    /// ---------------------------- Document ----------------------------

    struct DocumentImpl {
        DocumentImpl() = default;
        DocumentImpl(ryml::Tree tree)
            : m_tree(std::move(tree))
        { }
        ryml::Tree m_tree;
    };

    Document::~Document() {
        if (m_pImpl) {
            delete static_cast<DocumentImpl*>(m_pImpl);
            m_pImpl = nullptr;
        }
    }

    Document::Document(Document&& document) noexcept
        : m_pImpl(std::exchange(document.m_pImpl, {}))
        , m_path(std::exchange(document.m_path, {}))
    { }

    Document& Document::operator=(Document&& document) noexcept {
        m_pImpl = std::exchange(document.m_pImpl, {});
        m_path = std::exchange(document.m_path, {});
        return *this;
    }

    Document Document::Empty() {
        return Document();
    }

    Document Document::New() {
        Document yaml = Document();
        yaml.m_pImpl = new DocumentImpl();
        return yaml;
    }

    Node Document::AppendChild(const std::string& name) {
        if (!IsValid()) {
            SRHalt("Document::AppendChild() : document is not valid!");
            return { };
        }

        auto&& tree = static_cast<DocumentImpl*>(m_pImpl)->m_tree;
        auto&& nodeId = tree.append_child(tree.root_id());
        tree.get(nodeId)->m_key.scalar.assign(name.c_str());
        auto&& cref = tree.cref(nodeId);
        return Node(static_cast<void const*>(cref.m_tree), cref.m_id);
    }

    Document Document::Load(const Path& path) {
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
            return Empty();
        }

        yaml.m_pImpl = new DocumentImpl(std::move(tree));
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

        if (m_pImpl) {
            for (auto&& node : static_cast<DocumentImpl*>(m_pImpl)->m_tree.m_arena) {
                file << node;
            }
        }

        file.close();
        return true;
    }

    SR_NODISCARD Node Document::GetRoot() const {
        if (!IsValid()) {
            return Node();
        }

        ryml::ConstNodeRef root = static_cast<DocumentImpl*>(m_pImpl)->m_tree.crootref();
        if (!root.valid() || !root.has_children()) {
            SR_ERROR("Document::TryRoot() : failed to get root node from file!");
            return { };
        }

        return Node(static_cast<void const*>(root.m_tree), root.m_id);
    }
}