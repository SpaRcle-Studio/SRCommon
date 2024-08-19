//
// Created by Monika on 16.08.2024.
//

#include <Utils/Web/HTML/Tag.h>

#include <myhtml/api.h>

namespace SR_UTILS_NS::Web {
    HTMLTag MyHTMLTagToHTMLTag(size_t tag) {
        const myhtml_tag_id_t myhtmlTag = tag;
        static const std::unordered_map<myhtml_tag_id_t, HTMLTag> myhtmlToHTMLTag = {
            {MyHTML_TAG_DIV, HTMLTag::Div}, {MyHTML_TAG_SPAN, HTMLTag::Span}, {MyHTML_TAG_P, HTMLTag::P},
            {MyHTML_TAG_H1, HTMLTag::H1}, {MyHTML_TAG_H2, HTMLTag::H2}, {MyHTML_TAG_H3, HTMLTag::H3}, {MyHTML_TAG_H4, HTMLTag::H4}, {MyHTML_TAG_H5, HTMLTag::H5}, {MyHTML_TAG_H6, HTMLTag::H6},
            {MyHTML_TAG_A, HTMLTag::A}, {MyHTML_TAG_IMG, HTMLTag::Img}, {MyHTML_TAG_INPUT, HTMLTag::Input}, {MyHTML_TAG_BUTTON, HTMLTag::Button}, {MyHTML_TAG_FORM, HTMLTag::Form},
            {MyHTML_TAG_LABEL, HTMLTag::Label}, {MyHTML_TAG_SELECT, HTMLTag::Select}, {MyHTML_TAG_OPTION, HTMLTag::Option}, {MyHTML_TAG_TEXTAREA, HTMLTag::Textarea}, {MyHTML_TAG_TABLE, HTMLTag::Table},
            {MyHTML_TAG_TR, HTMLTag::Tr}, {MyHTML_TAG_TD, HTMLTag::Td}, {MyHTML_TAG_TH, HTMLTag::Th}, {MyHTML_TAG_THEAD, HTMLTag::Thead}, {MyHTML_TAG_TBODY, HTMLTag::Tbody},
            {MyHTML_TAG_TFOOT, HTMLTag::Tfoot}, {MyHTML_TAG_UL, HTMLTag::Ul}, {MyHTML_TAG_OL, HTMLTag::Ol}, {MyHTML_TAG_LI, HTMLTag::Li}, {MyHTML_TAG_DL, HTMLTag::Dl},
            {MyHTML_TAG_DT, HTMLTag::Dt}, {MyHTML_TAG_DD, HTMLTag::Dd}, {MyHTML_TAG_BR, HTMLTag::Br}, {MyHTML_TAG_HR, HTMLTag::Hr}, {MyHTML_TAG_META, HTMLTag::Meta}, {MyHTML_TAG_LINK, HTMLTag::Link},
            {MyHTML_TAG_STYLE, HTMLTag::Style}, {MyHTML_TAG_SCRIPT, HTMLTag::Script}, {MyHTML_TAG_TITLE, HTMLTag::Title}, {MyHTML_TAG_HEAD, HTMLTag::Head}, {MyHTML_TAG_BODY, HTMLTag::Body},
            {MyHTML_TAG_HTML, HTMLTag::Html}, {MyHTML_TAG__TEXT, HTMLTag::Text},
        };

        if (const auto it = myhtmlToHTMLTag.find(myhtmlTag); it != myhtmlToHTMLTag.end()) {
            return it->second;
        }
        return HTMLTag::Undefined;
    }
}