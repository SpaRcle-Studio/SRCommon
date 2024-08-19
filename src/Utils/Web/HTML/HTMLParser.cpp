//
// Created by Monika on 15.08.2024.
//

#include <Utils/Web/HTML/HTMLParser.h>
#include <Utils/Debug.h>

#include <myhtml/api.h>
#include <Utils/FileSystem/FileSystem.h>

namespace SR_UTILS_NS::Web {
    HTMLPage::Ptr HTMLParser::Parse(const Path& path) {
        const std::string htmlData = SR_UTILS_NS::FileSystem::ReadBinaryAsString(path);
        if (htmlData.empty()) {
            SR_ERROR("HTMLParser::Parse() : failed to read HTML file: {}", path.c_str());
            return nullptr;
        }

        myhtml_t* pMyhtml = myhtml_create();
        myhtml_init(pMyhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);

        myhtml_tree_t* pTree = myhtml_tree_create();
        myhtml_tree_init(pTree, pMyhtml);

        myhtml_tree_parse_flags_set(pTree,
                            static_cast<myhtml_tree_parse_flags_t>(MyHTML_TREE_PARSE_FLAGS_SKIP_WHITESPACE_TOKEN |
                                                                   MyHTML_TREE_PARSE_FLAGS_WITHOUT_DOCTYPE_IN_TREE));

        myhtml_parse(pTree, MyENCODING_UTF_8, htmlData.c_str(), htmlData.size());

        const HTMLPage::Ptr pPage = ParseTree(pTree);
        if (!pPage) {
            SR_ERROR("HTMLParser::Parse() : failed to parse HTML file: {}", path.c_str());
        }

        myhtml_tree_destroy(pTree);
        myhtml_destroy(pMyhtml);

        return pPage;
    }

    std::string HTMLParser::DebugNodeTostring(const HTMLNode* pNode, uint16_t depth) const {
        std::string result;

        if (!pNode) {
            SR_ERROR("HTMLParser::DebugPrintNode() : invalid node");
            return result;
        }

        result.reserve(1024);

        if (depth > 0) {
            result.append(std::string(depth, '\t'));
        }

        //if (pNode->GetNodeName().empty()) {
        //    const SR_UTILS_NS::StringAtom tag = HTMLTagToStringAtom(pNode->GetTag());
        //    result.append(SR_FORMAT(">: {}\n", tag.c_str()));
        //}
        //else {
        //    result.append(SR_FORMAT("<{}", pNode->GetNodeName()));
        //}

        if (pNode->GetTag() == HTMLTag::Text) {
            result.append(SR_FORMAT("<{}", pNode->GetNodeName()));
        }
        else {
            result.append(SR_FORMAT("<{}", pNode->GetNodeName()));
        }

        for (const auto& pAttribute : pNode->GetAttributes()) {
            result.append(SR_FORMAT(" {}", DebugAttributeTostring(pAttribute)));
        }

        if (!pNode->GetText().empty()) {
            result.append(SR_FORMAT(">: {}\n", pNode->GetText()));
        }
        else {
            result.append(">\n");
        }

        for (const auto& pChild : pNode->GetChildren()) {
            result.append(DebugNodeTostring(pChild, depth + 1));
        }

        if (depth > 0) {
            result.append(std::string(depth, '\t'));
        }

        result.append(SR_FORMAT("</{}>\n", pNode->GetNodeName().c_str()));
        return result;
    }

    std::string HTMLParser::DebugAttributeTostring(const HTMLAttribute* pAttribute) const {
        return SR_FORMAT("{}=\"{}\"", pAttribute->GetName(), pAttribute->GetValue());
    }

    std::string HTMLParser::DebugPageTostring(const HTMLPage::Ptr& pPage) const {
        std::string result;

        if (!pPage) {
            SR_ERROR("HTMLParser::DebugPrintPage() : invalid page");
            return result;
        }

        result.reserve(1024);
        result.append(DebugNodeTostring(pPage->GetHead(), 0));
        result.append(DebugNodeTostring(pPage->GetBody(), 0));
        return result;
    }

    HTMLPage::Ptr HTMLParser::ParseTree(myhtml_tree* pTree) const {
        myhtml_tree_node_t* pNode = myhtml_tree_get_document(pTree);
        if (!pNode) {
            SR_ERROR("HTMLParser::ParseTree() : failed to get document node");
            return nullptr;
        }

        auto&& pPage = HTMLPage::MakeShared();
        if (!ParseNode(pTree, pNode, pPage.Get(), nullptr)) {
            SR_ERROR("HTMLParser::ParseTree() : failed to parse node");
            return nullptr;
        }

        return pPage;
    }

    bool HTMLParser::ParseNode(myhtml_tree* pTree, myhtml_tree_node* pRawNode, HTMLPage* pPage, HTMLNode* pNodeParent) const {
        if (!pRawNode || !pTree) {
            return false;
        }

        while (pRawNode) {
            const myhtml_tag_id_t tagId = myhtml_node_tag_id(pRawNode);

            if ((tagId == MyHTML_TAG_HTML || tagId == MyHTML_TAG__UNDEF) && !pNodeParent) {
                SRAssert(!pPage->GetHead() && !pPage->GetBody());
                pRawNode = myhtml_node_child(pRawNode);
                continue;
            }

            if (tagId == MyHTML_TAG__COMMENT) {
                pRawNode = myhtml_node_next(pRawNode);
                continue;
            }

            auto&& pNode = new HTMLNode();

            pNode->SetTag(MyHTMLTagToHTMLTag(tagId));
            pNode->SetParent(pNodeParent);

            if (tagId == MyHTML_TAG__UNDEF) {
                pNode->SetNodeName(myhtml_tag_name_by_id(pTree, tagId, nullptr));
            }
            else {
                SR_NOOP;
            }

            if (tagId == MyHTML_TAG__TEXT) {
                pNode->SetText(myhtml_node_text(pRawNode, nullptr));
            }

            if (pNodeParent) {
                pNodeParent->AddChild(pNode);
            }
            else if (tagId == MyHTML_TAG_HEAD) {
                SRAssert(!pPage->GetHead());
                pPage->SetHead(pNode);
            }
            else if (tagId == MyHTML_TAG_BODY) {
                SRAssert(!pPage->GetBody());
                pPage->SetBody(pNode);
            }
            else {
                SR_WARN("HTMLParser::ParseNode() : cannot add node to parent with name: {}", pNode->GetNodeName().c_str());
                delete pNode;
                pRawNode = myhtml_node_next(pRawNode);
                continue;
            }

            ParseNodeAttributes(pRawNode, pNode);

            ParseNode(pTree, myhtml_node_child(pRawNode), pPage, pNode);

            pRawNode = myhtml_node_next(pRawNode);
        }

        return true;
    }

    bool HTMLParser::ParseNodeAttributes(myhtml_tree_node* pNode, HTMLNode* pClass) const {
        if (!pNode || !pClass) {
            SRHalt("HTMLParser::ParseNodeAttributes() : invalid arguments");
            return false;
        }
        myhtml_tree_attr_t* pRawAttribute = myhtml_node_attribute_first(pNode);

        while (pRawAttribute) {
            if (const char* name = myhtml_attribute_key(pRawAttribute, nullptr)) {
                if (const char* value = myhtml_attribute_value(pRawAttribute, nullptr)) {
                    auto&& pAttribute = new HTMLAttribute();
                    pAttribute->SetName(name);
                    pAttribute->SetValue(value);
                    pClass->AddAttribute(pAttribute);
                }
            }

            pRawAttribute = myhtml_attribute_next(pRawAttribute);
        }

        return true;
    }
}
