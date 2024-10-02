//
// Created by Monika on 15.08.2024.
//

#include <Utils/Web/HTML/HTMLParser.h>
#include <Utils/Web/CSS/CSSParser.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Debug.h>
#include <Utils/Resources/ResourceManager.h>

#include <myhtml/api.h>

namespace SR_UTILS_NS::Web {
    HTMLPage::Ptr HTMLParser::Parse(const Path& path) {
        const SR_UTILS_NS::Path& fullPath = SR_UTILS_NS::ResourceManager::Instance().GetResPath().Concat(path);
        const std::string htmlData = SR_UTILS_NS::FileSystem::ReadBinaryAsString(fullPath);
        if (htmlData.empty()) {
            SR_ERROR("HTMLParser::Parse() : failed to read HTML file: {}", fullPath.c_str());
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

        HTMLPage::Ptr pPage = ParseTree(pTree);
        if (!pPage) {
            SR_ERROR("HTMLParser::Parse() : failed to parse HTML file: {}", fullPath.c_str());
        }

        pPage->AddPath(path);

        if (!ProcessCSS(pPage.Get())) {
            SR_ERROR("HTMLParser::Parse() : failed to process CSS");
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

        for (const auto& attributeId : pNode->GetAttributes()) {
            auto&& pAttribute = pNode->GetPage()->GetAttributeById(attributeId);
            result.append(SR_FORMAT(" {}", DebugAttributeTostring(pAttribute)));
        }

        if (!pNode->GetText().empty()) {
            result.append(SR_FORMAT(">: {}\n", pNode->GetText()));
        }
        else {
            result.append(">\n");
        }

        for (const auto& childId : pNode->GetChildren()) {
            auto&& pChild = pNode->GetPage()->GetNodeById(childId);
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
        if (!ParseNode(pTree, pNode, pPage.Get(), SR_ID_INVALID)) {
            SR_ERROR("HTMLParser::ParseTree() : failed to parse node");
            return nullptr;
        }

        return pPage;
    }

    bool HTMLParser::ParseNode(myhtml_tree* pTree, myhtml_tree_node* pRawNode, HTMLPage* pPage, uint64_t parentNodeId) const {
        if (!pRawNode || !pTree) {
            return false;
        }

        while (pRawNode) {
            const myhtml_tag_id_t tagId = myhtml_node_tag_id(pRawNode);

            if ((tagId == MyHTML_TAG_HTML || tagId == MyHTML_TAG__UNDEF) && parentNodeId == SR_ID_INVALID) {
                SRAssert(!pPage->GetHead() && !pPage->GetBody());
                pRawNode = myhtml_node_child(pRawNode);
                continue;
            }

            if (tagId == MyHTML_TAG__COMMENT) {
                pRawNode = myhtml_node_next(pRawNode);
                continue;
            }

            auto&& pNode = pPage->AllocateNode();

            if (tagId == MyHTML_TAG__UNDEF) {
                pNode->SetNodeName(myhtml_tag_name_by_id(pTree, tagId, nullptr));
            }
            else if (const HTMLTag tag = MyHTMLTagToHTMLTag(tagId); tag != HTMLTag::Undefined) {
                pNode->SetTag(tag);
            }
            else {
                SRHalt("HTMLParser::ParseNode() : invalid tag id: {}", tagId);
            }

            if (tagId == MyHTML_TAG__TEXT) {
                pNode->SetText(myhtml_node_text(pRawNode, nullptr));
            }

            if (parentNodeId != SR_ID_INVALID) {
                pNode->SetParent(parentNodeId);
                pPage->GetNodeById(parentNodeId)->AddChild(pNode);
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
                pPage->FreeNode(pNode);
                pRawNode = myhtml_node_next(pRawNode);
                continue;
            }

            ParseNodeAttributes(pRawNode, pNode);

            ParseNode(pTree, myhtml_node_child(pRawNode), pPage, pNode->GetId());

            pRawNode = myhtml_node_next(pRawNode);
        }

        return true;
    }

    bool HTMLParser::ParseNodeAttributes(myhtml_tree_node* pRawNode, HTMLNode* pNode) const {
        if (!pRawNode || !pNode) {
            SRHalt("HTMLParser::ParseNodeAttributes() : invalid arguments");
            return false;
        }
        myhtml_tree_attr_t* pRawAttribute = myhtml_node_attribute_first(pRawNode);

        while (pRawAttribute) {
            if (const char* name = myhtml_attribute_key(pRawAttribute, nullptr)) {
                if (const char* value = myhtml_attribute_value(pRawAttribute, nullptr)) {
                    auto&& pAttribute = pNode->GetPage()->AllocateAttribute();
                    pAttribute->SetName(name);
                    pAttribute->SetValue(value);
                    pNode->AddAttribute(pAttribute);
                }
            }

            pRawAttribute = myhtml_attribute_next(pRawAttribute);
        }

        return true;
    }

    bool HTMLParser::ProcessCSS(HTMLPage* pPage) const {
        SR_TRACY_ZONE;

        if (!pPage->GetHead()) {
            SR_WARN("HTMLParser::ProcessCSS() : no head node!");
            return false;
        }

        for (const auto& nodeId : pPage->GetHead()->GetChildren()) {
            auto&& pNode = pPage->GetNodeById(nodeId);
            if (pNode->GetTag() == HTMLTag::Style) {
                HTMLAttribute* pRelAttribute = pNode->GetAttributeByName("type");
                if (pRelAttribute && pRelAttribute->GetValue() != "text/css") {
                    continue;
                }

                if (pNode->GetChildren().size() != 1) {
                    SR_WARN("HTMLParser::ProcessCSS() : invalid CSS node!");
                    continue;
                }

                pNode = pPage->GetNodeById(pNode->GetChildren().front());
                const std::string css = pNode->GetText();
                if (css.empty()) {
                    SR_WARN("HTMLParser::ProcessCSS() : empty CSS");
                    continue;
                }
                if (auto&& pStyle = CSSParser::Instance().Parse(css)) {
                    pPage->AddStyle(pStyle);
                }
                continue;
            }

            if (pNode->GetTag() == HTMLTag::Link) {
                HTMLAttribute* pRelAttribute = pNode->GetAttributeByName("rel");
                if (!pRelAttribute || pRelAttribute->GetValue() != "stylesheet") {
                    continue;
                }

                HTMLAttribute* pAttribute = pNode->GetAttributeByName("href");
                if (!pAttribute) {
                    SR_WARN("HTMLParser::ProcessCSS() : empty href!");
                    continue;
                }

                const Path cssPath = SR_UTILS_NS::ResourceManager::Instance().GetResPath().Concat(pAttribute->GetValue());
                if (auto&& pStyle = CSSParser::Instance().Parse(cssPath)) {
                    pPage->AddStyle(pStyle);
                    pPage->AddPath(pAttribute->GetValue());
                }
                continue;
            }
        }

        std::function <void(HTMLNode*)> processChildren;

        processChildren = [&](HTMLNode* pNode) {
            if (pNode->GetTag() == HTMLTag::Undefined) {
                if (auto&& style = pPage->GetTagStyle(pNode->GetNodeName())) {
                    pNode->SetStyle(style.value());
                }
            }
            else {
                if (auto&& style = pPage->GetTagStyle(HTMLTagToStringAtom(pNode->GetTag()))) {
                    pNode->SetStyle(style.value());
                }
                auto&& style = pNode->GetStyle();
                if (pNode->GetTag() == HTMLTag::Body) {
                    if (style.display.IsDefault()) {
                        style.display = DEFAULT_CSS_DISPLAY_BODY;
                        style.display.SetAsDefault();
                    }
                }
            }

            for (const auto& attributeId : pNode->GetAttributes()) {
                auto&& pAttribute = pNode->GetPage()->GetAttributeById(attributeId);
                if (pAttribute->GetName() == "class") {
                    if (const auto& style = pPage->GetClassStyle(pAttribute->GetValue())) {
                        pNode->SetStyle(style.value());
                        break;
                    }
                }
            }

            for (const auto& childId : pNode->GetChildren()) {
                auto&& pChild = pNode->GetPage()->GetNodeById(childId);
                processChildren(pChild);
            }
        };

        processChildren(pPage->GetBody());

        return true;
    }
}
