//
// Created by Monika on 03.09.2021.
//

#include <Utils/Resources/Xml.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS {
    std::string Xml::Attribute::ToString() const {
        if (!CheckError(SR_FORMAT("Attribute::ToStringAtom() : attribute isn't valid! Name: {}", m_attribute.name()))) {
            return std::string();
        }
        else
            return m_attribute.as_string();
    }

    int32_t Xml::Attribute::ToInt() const {
        if (!CheckError(SR_FORMAT("Attribute::ToInt() : attribute isn't valid! Name: {}", m_attribute.name()))) {
            return 0;
        }
        else
            return m_attribute.as_int();
    }

    float_t Xml::Attribute::ToFloat() const {
        if (!CheckError(SR_FORMAT("Attribute::ToFloat() : attribute isn't valid! Name: {}", m_attribute.name()))) {
            return 0.f;
        }
        else
            return m_attribute.as_float();
    }

    double_t Xml::Attribute::ToDouble() const {
        if (!CheckError(SR_FORMAT("Attribute::ToFloat() : attribute isn't valid! Name: {}", m_attribute.name()))) {
            return 0.f;
        }
        else
            return m_attribute.as_double();
    }

    bool Xml::Attribute::ToBool() const {
        if (!CheckError(SR_FORMAT("Attribute::ToBool() : attribute isn't valid! Name: {}", m_attribute.name()))) {
            return false;
        }
        else
            return m_attribute.as_bool();
    }

    std::string Xml::Attribute::ToString(const std::string &def) const {
        return m_valid ? m_attribute.as_string() : def;
    }

    int32_t Xml::Attribute::ToInt(int32_t def) const {
        return m_valid ? m_attribute.as_int() : def;
    }

    float_t Xml::Attribute::ToFloat(float_t def) const {
        return m_valid ? m_attribute.as_float() : def;
    }

    bool Xml::Attribute::ToBool(bool def) const {
        return m_valid ? m_attribute.as_bool() : def;
    }

    int64_t Xml::Attribute::ToInt64(int64_t def) const {
        return m_valid ? m_attribute.as_llong() : def;
    }

    int64_t Xml::Attribute::ToInt64() const {
        if (!CheckError("Attribute::ToInt64() : attribute isn't valid!"))
            return 0;
        else
            return m_attribute.as_llong();
    }

    uint64_t Xml::Attribute::ToUInt64() const {
        if (!CheckError("Attribute::ToInt64() : attribute isn't valid!"))
            return 0;
        else
            return m_attribute.as_ullong();
    }

    uint32_t Xml::Attribute::ToUInt() const {
        if (!CheckError("Attribute::ToUInt() : attribute isn't valid!"))
            return 0;
        else
            return m_attribute.as_uint();
    }

    uint32_t Xml::Attribute::ToUInt(uint32_t def) const {
        return m_valid ? m_attribute.as_uint() : def;
    }

    uint64_t Xml::Attribute::ToUInt64(uint64_t def) const {
        return m_valid ? m_attribute.as_ullong() : def;
    }

    Xml::Attribute::Attribute()
        : m_attribute()
        , m_valid(false)
    { }

    Xml::Attribute::Attribute(const pugi::xml_attribute &attribute) {
        m_attribute = attribute;
        m_valid = !m_attribute.empty();
    }

    bool Xml::Attribute::CheckError(const std::string &msg) const {
        if (m_valid) {
            return true;
        }
        else {
            SRAssert2(false, msg);
            g_xml_last_error = -1;
            return false;
        }
    }

    Xml::Attribute::operator bool() const {
        return Valid();
    }

    bool Xml::Attribute::Valid() const {
        return m_valid;
    }

    Xml::Attribute::~Attribute() = default;

    Xml::Document Xml::Document::Load(const Path &path) {
        SR_TRACY_ZONE;
        SR_TRACY_TEXT_N("Path", path.ToStringRef());

        auto xml = Document::New();

        std::string fileData;
        if (!SR_UTILS_NS::FileSystem::ReadFile(path, fileData)) {
            SR_ERROR("Document::Load() : file not exists! \n\tPath: " + path.ToString());
            return Document(); /// NOLINT
        }

        if (pugi::xml_parse_result result = xml.m_document->load_string(fileData.c_str())) {
            xml.m_valid = true;
            xml.m_path = std::move(path.ToString());
        }
        else {
            SR_ERROR("Document::Load() : failed to load xml! \n\tPath: " + path.ToString() + "\n\tDescription: " + std::string(result.description()));
            Xml::g_xml_last_error = -3;
            xml.m_valid = false;
        }

        return xml;
    }

    std::string Xml::Document::Dump() const {
        if (!Valid())
            return std::string();

        std::ostringstream stream;
        m_document->save(stream, PUGIXML_TEXT("    "));

        return stream.str();
    }

    Xml::Document::operator bool() const {
        return Valid();
    }

    Xml::Node Xml::Document::DocumentElement() const {
        return Node(m_document->document_element());
    }

    Xml::Node Xml::Document::TryRoot() const {
        if (!Valid()) {
            return Node();
        }

        return Node(m_document->root());
    }

    Xml::Node Xml::Document::Root() const {
        if (!Valid()) {
            SRAssert2(false,"Document::Root() : document is not valid!");
            g_xml_last_error = -2;
            return Node();
        }
        return Node(m_document->root());
    }

    bool Xml::Document::Save(const Path &path) const {
        if (!path.Exists()) {
            path.Create();
        }

        std::string document = Dump();
        if (document.empty()) {
            SR_ERROR("Document::Save() : document is empty!");
            return false;
        }

        if (!SR_UTILS_NS::FileSystem::WriteToFile(path.ToStringRef(), document)) {
            SR_ERROR("Document::Save() : failed to save document! \n\tPath: " + path.ToString());
            return false;
        }

        return true;
    }

    Xml::Node Xml::Document::AppendChild(const std::string &name) {
        if (!m_valid) {
            SRAssert2(false,"Document::AppendChild() : document is not valid!");
            g_xml_last_error = -2;
            return Node();
        }

        auto node = m_document->append_child(name.c_str());
        return Node(node);
    }

    int32_t Xml::Document::GetLastError() {
        auto last = Xml::g_xml_last_error;
        Xml::g_xml_last_error = 0;
        return last;
    }

    Xml::Document Xml::Document::New() {
        auto xml = Document();
        xml.m_valid = true;
        xml.m_path = "None";
        xml.m_document = new pugi::xml_document();
        return xml;
    }

    Xml::Document Xml::Document::Empty() {
        return Document();
    }

    Xml::Document::Document() {
        m_valid = false;
    }

    Xml::Document::Document(Xml::Document &&document) noexcept
        : m_document(std::exchange(document.m_document, {}))
        , m_valid(std::exchange(document.m_valid, {}))
        , m_path(std::exchange(document.m_path, {}))
    { }

    Xml::Document::~Document() {
        if (m_document) {
            delete m_document;
        }
    }

    Xml::Document &Xml::Document::operator=(Xml::Document &&document) noexcept {
        m_document = std::exchange(document.m_document, {});
        m_valid = std::exchange(document.m_valid, {});
        m_path = std::exchange(document.m_path, {});
        return *this;
    }

    bool Xml::Document::Valid() const {
        return m_valid;
    }

    Xml::Node::Node()
        : m_node()
        , m_valid(false)
    { }

    Xml::Document Xml::Node::ToDocument() const {
        auto doc = Document::New();
        doc.Root().AppendChild(*this);
        return doc;
    }

    Xml::Node Xml::Node::AppendChild(const std::string &name) {
        if (!m_valid) {
            SRAssert2(false,"Node::AppendChild() : node is not valid!");
            g_xml_last_error = -2;
            return Node();
        }

        return Node(m_node.append_child(name.c_str()));
    }

    std::vector<Xml::Node> Xml::Node::GetNodes() const {
        if (!m_valid) {
            SRAssert2(false,"Node::GetNodes() : node is not valid!");
            g_xml_last_error = -2;
            return {};
        }

        auto&& nodes = std::vector<Node>();
        for (const auto child : m_node.children())
            nodes.emplace_back(Node(child));

        return nodes;
    }

    std::vector<Xml::Node> Xml::Node::TryGetNodes(const std::string &name) const {
        if (Valid())
            return GetNodes(name);

        return std::vector<Node>();
    }

    std::vector<Xml::Node> Xml::Node::TryGetNodes() const {
        if (Valid())
            return GetNodes();

        return std::vector<Node>();
    }


    std::vector<Xml::Node> Xml::Node::GetNodes(const std::string &name) const {
        if (!m_valid) {
            SRAssert2(false,"Node::GetNodes() : node is not valid!");
            g_xml_last_error = -2;
            return {};
        }

        auto nodes = std::vector<Node>();
        for (const auto child : m_node.children())
            if (std::string(child.name()) == name)
                nodes.emplace_back(Node(child));

        return nodes;
    }

    Xml::Node Xml::Node::AppendChild(const Xml::Node& node) {
        return Xml::Node(m_node.append_copy(node.m_node));
    }

    Xml::Node Xml::Node::GetNode(const std::string &name) const  {
        if (!m_valid) {
            SRAssert2(false, "Node::GetNode() : node is not valid!");
            g_xml_last_error = -2;
            return Node();
        }

        return Node(m_node.child(name.c_str()));
    }

    Xml::Node Xml::Node::TryGetNode(const std::string &name) const  {
        return m_valid ? Node(m_node.child(name.c_str())) : Node();
    }

    bool Xml::Node::HasAttribute(const std::string &name) const {
        return m_valid ? !m_node.attribute(name.c_str()).empty() : false;
    }

    Xml::Attribute Xml::Node::TryGetAttribute(const std::string &name) const {
        return m_valid ? Attribute(m_node.attribute(name.c_str())) : Attribute();
    }

    Xml::Attribute Xml::Node::GetAttribute(const std::string &name) const {
        if (!m_valid) {
            SRAssert2(false, "Node::GetAttribute() : node is not valid!");
            g_xml_last_error = -4;
            return Attribute();
        }

        return Attribute(m_node.attribute(name.c_str()));
    }

    std::string_view Xml::Node::NameView() const {
        if (!m_valid) {
            SRAssert2(false, "Node::Name() : node is not valid!");
            g_xml_last_error = -4;
            return {};
        }

        return m_node.name();
    }

    std::string Xml::Node::Name() const {
        if (!m_valid) {
            SRAssert2(false, "Node::Name() : node is not valid!");
            g_xml_last_error = -4;
            return {};
        }

        return m_node.name();
    }

    bool Xml::Node::Valid() const {
        return m_valid;
    }

    Xml::Node::operator bool() const {
        return Valid();
    }

    Xml::Node::Node(pugi::xml_node node) {
        m_node = node;
        m_valid = !m_node.empty();
    }

    Xml::Node Xml::Node::Empty() {
        return Node();
    }

    Xml::Node Xml::Node::AppendNode(const std::string &name) {
        return AppendChild(name);
    }

    Xml::Node Xml::Node::AppendNode(const Xml::Node &node) {
        return AppendChild(node);
    }

    Xml::Node::~Node() = default;

    void Xml::AppendColorNode(Xml::Node &node, const Math::FColor &color) {
        node.AppendChild("Color")
            .NAppendAttribute("r", color.r * 255.f)
            .NAppendAttribute("g", color.g * 255.f)
            .NAppendAttribute("b", color.b * 255.f)
            .NAppendAttribute("a", color.a * 255.f);
    }
}