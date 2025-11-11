//
// Created by innerviewer on 2024-06-08.
//

#include <Utils/Resources/Yaml.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Debug.h>

#include <rapidyaml/src/ryml.hpp>

namespace SR_UTILS_NS::Yaml {
    std::string Node::Name() const {
        if (!IsValid() || !GetTreeImpl()) {
            SRHalt("Node::Name() : node is not valid!");
            return { };
        }

        return ryml::ConstNodeRef(static_cast<ryml::Tree const*>(GetTreeImpl()), m_id).key().str;
    }

    std::string Node::GetValue() const {
        if (!IsValid() || !GetTreeImpl()) {
            SRHalt("Node::GetValue() : node is not valid!");
            return { };
        }

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(GetTreeImpl()), m_id);
        return { node.val().begin(), node.val().end() };
    }

    std::string_view Node::GetValueView() const {
        if (!IsValid() || !GetTreeImpl()) {
            SRHalt("Node::GetValueView() : node is not valid!");
            return {};
        }

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(GetTreeImpl()), m_id);
        return std::string_view(node.val().begin(), node.val().size());
    }

    std::string Node::GetKey() const {
        if (!IsValid() || !GetTreeImpl()) {
            SRHalt("Node::GetKey() : node is not valid!");
            return { };
        }

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(GetTreeImpl()), m_id);
        return { node.key().begin(), node.key().end() };
    }

    std::string_view Node::GetKeyView() const {
        if (!IsValid() || !GetTreeImpl()) {
            SRHalt("Node::GetKeyView() : node is not valid!");
            return {};
        }

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(GetTreeImpl()), m_id);
        return std::string_view(node.key().begin(), node.key().size());
    }

    std::vector<Node> Node::GetChildren() const {
        if (!IsValid() || !GetTreeImpl()) {
            SRHalt("Node::GetChildren() : node is not valid!");
            return { };
        }

        std::vector<Node> children;

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(GetTreeImpl()), m_id);
        children.reserve(node.num_children());

        for (auto&& child : node.children()) {
            children.emplace_back(Node(static_cast<void const*>(child.m_tree), child.m_id));
        }

        return children;
    }

    uint16_t Node::GetId() const {
        if (!IsValid() || !GetTreeImpl()) {
            SRHalt("Node::GetId() : node is not valid!");
            return { };
        }

        return m_id;
    }

    SR_NODISCARD Node Node::GetChild(const std::string_view& name) const {
        if (!IsValid() || !GetTreeImpl()) {
            return { };
        }

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(GetTreeImpl()), m_id);
        if (!node.has_child(name.data())) {
            return { };
        }

        auto&& child = node.find_child(name.data());
        if (!child.valid()) {
            return { };
        }

        return Node(static_cast<void const*>(child.m_tree), child.m_id);
    }

    Node Node::AppendChild(const std::string_view& name)
    {
        if (!IsValid() || !GetTreeImpl()) {
            SRHalt("Node::AppendChild() : node is not valid!");
            return { };
        }

        auto&& node = ryml::NodeRef(static_cast<ryml::Tree*>(GetTreeImpl()), m_id);
        if (!node.valid()) {
            SRHalt("Node::AppendChild() : failed to get node!");
            return { };
        }

        auto&& child = node.append_child();
        child.set_key(name.data());
        return Node(static_cast<void*>(child.tree()), child.id());
    }

    std::string_view Node::NameView() const {
        if (!IsValid() || !GetTreeImpl()) {
            SRHalt("Node::NameView() : node is not valid!");
            return {};
        }

        auto&& node = ryml::ConstNodeRef(static_cast<ryml::Tree const*>(GetTreeImpl()), m_id);
        return node.key().str;
    }

    void Node::SetValue(const std::string_view& value)
    {
        if (!IsValid() || !GetTreeImpl()) {
            SRHalt("Node::SetValue() : node is not valid!");
            return;
        }

        auto&& node = ryml::NodeRef(static_cast<ryml::Tree*>(GetTreeImpl()), m_id);
        node.set_val(value.data());
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

    Node Document::AppendChild(const std::string_view& name) {
        if (!IsValid()) {
            SRHalt("Document::AppendChild() : document is not valid!");
            return { };
        }

        auto&& tree = static_cast<DocumentImpl*>(m_pImpl)->m_tree;
        auto&& nodeId = tree.append_child(tree.root_id());
        tree.get(nodeId)->m_key.scalar.assign(name.data());
        auto&& ref = tree.ref(nodeId);
        return Node(static_cast<void*>(ref.tree()), ref.id());
    }

    Document Document::Load(const Path& path) {
        SR_TRACY_ZONE;
        SR_TRACY_TEXT_N("Path", path.ToStringRef());

        auto&& contents = SR_PLATFORM_NS::ReadFile(path);
        if (!contents) {
            SR_ERROR("Document::Load() : failed to open document!\n\tPath: {}", path);
            return { };
        }
        Document yaml = Document::New();

        if (contents.value().empty()) {
            SR_ERROR("Document::Load() : failed to read YAML contents!\n\tPath: {}", path);
            return { };
        }

        ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(contents.value()));
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
            SRHalt("Document::GetRoot() : document is not valid!");
            return Node();
        }

        ryml::ConstNodeRef root = static_cast<DocumentImpl*>(m_pImpl)->m_tree.crootref();
        if (!root.valid()) {
            SRHalt("Document::GetRoot() : failed to get root node from file!");
            return { };
        }

        return Node(static_cast<void const*>(root.m_tree), root.m_id);
    }

    Node Document::GetRoot() {
        if (!IsValid()) {
            SRHalt("Document::GetRoot() : document is not valid!");
            return Node();
        }

        ryml::NodeRef root = static_cast<DocumentImpl*>(m_pImpl)->m_tree.rootref();
        root |= ryml::MAP;

        if (!root.valid()) {
            SRHalt("Document::GetRoot() : failed to get root node from file!");
            return { };
        }

        return Node(static_cast<void*>(root.tree()), root.id());
    }

    std::string Document::Dump() const {
        if (!IsValid()) {
            SRHalt("Document::Dump() : document is not valid!");
            return {};
        }

        std::stringstream output;

        for (auto&& node : static_cast<DocumentImpl*>(m_pImpl)->m_tree.m_arena) {
            output << node;
        }

        return output.str();
    }
}
