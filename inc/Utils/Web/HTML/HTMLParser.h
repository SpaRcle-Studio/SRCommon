//
// Created by Monika on 15.08.2024.
//

#ifndef SR_COMMON_WEB_HTML_PARSER_H
#define SR_COMMON_WEB_HTML_PARSER_H

#include <Utils/Common/Singleton.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Web/HTML/HTML.h>

struct myhtml_tree;
struct myhtml_tree_node;

namespace SR_UTILS_NS::Web {
    class HTMLParser : public Singleton<HTMLParser> {
        SR_REGISTER_SINGLETON(HTMLParser)
    public:
        SR_NODISCARD HTMLPage::Ptr Parse(const Path& path);
        std::string DebugPageTostring(const HTMLPage::Ptr& pPage) const;

    private:
        SR_NODISCARD HTMLPage::Ptr ParseTree(myhtml_tree* pTree) const;
        bool ParseNode(myhtml_tree* pTree, myhtml_tree_node* pRawNode, HTMLPage* pPage, HTMLNode* pNode) const;
        bool ParseNodeAttributes(myhtml_tree_node* pNode, HTMLNode* pClass) const;

        std::string DebugNodeTostring(const HTMLNode* pNode, uint16_t depth) const;
        std::string DebugAttributeTostring(const HTMLAttribute* pAttribute) const;


    };
}

#endif //SR_COMMON_WEB_HTML_PARSER_H
