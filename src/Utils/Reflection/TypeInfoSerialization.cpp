//
// Created by Monika on 10.08.2026.
//

#include <Utils/Reflection/TypeInfoSerialization.h>
#include <Utils/Reflection/ReflectedType.h>

namespace SR_UTILS_NS::Reflection {
    struct TypeParser {
        StringView source;
        SizeType pos = 0;

        void SkipSpaces();

        bool Consume(char c) {
            SkipSpaces();

            if (pos < source.size() && source[pos] == c) {
                ++pos;
                return true;
            }

            return false;
        }

        StringView ReadBaseType() {
            const SizeType start = pos;

            while (pos < source.size()) {
                const char c = source[pos];

                if (c == '[' || c == '(' || c == ')' || c == ']' || c == ',') {
                    break;
                }

                ++pos;
            }

            return source.substr(start, pos - start);
        }

        TypeInfo* ParseType();
    };

    void TypeParser::SkipSpaces() {
        while (pos < source.size() && std::isspace(static_cast<unsigned char>(source[pos]))) {
            ++pos;
        }
    }

    TypeInfo* TypeParser::ParseType() {
        SkipSpaces();

        TypeInfo* pTypeInfo = AllocateTypeInfo();
        StringView baseType = ReadBaseType();

        const SizeType dotPos = baseType.find('.');
        if (dotPos == StringView::npos) {
            return nullptr;
        }

        pTypeInfo->category = EnumReflector::FromString<ReflectedCategoryType>(baseType.substr(0, dotPos));
        pTypeInfo->detailedType = baseType.substr(dotPos + 1);

        if (Consume('(')) {
            SkipSpaces();
            const SizeType start = pos;
            while (pos < source.size() && std::isdigit(static_cast<unsigned char>(source[pos]))) {
                ++pos;
            }

            StringView sizeStr = source.substr(start, pos - start);
            pTypeInfo->detailedSize = static_cast<uint8_t>(std::stoull(std::string(sizeStr)));

            if (!Consume(')')) {
                return nullptr;
            }
        }

        if (Consume('[')) {
            uint32_t index = 0;

            while (true) {
                if (index >= 2) {
                    return nullptr;
                }

                pTypeInfo->pNext[index++] = ParseType();

                if (!pTypeInfo->pNext[index - 1]) {
                    return nullptr;
                }

                SkipSpaces();

                if (Consume(',')) {
                    continue;
                }

                if (Consume(']')) {
                    break;
                }

                return nullptr;
            }
        }

        return pTypeInfo;
    }

    TypeInfo* LoadTypeInfo(StringView type) {
        TypeParser parser{ type };
        return parser.ParseType();
    }

    void SaveTypeInfoInternal(String& type, const TypeInfo* pTypeInfo) { /// NOLINT(misc-no-recursion)
        if (!pTypeInfo) {
            return;
        }

        type += EnumReflector::ToStringAtom(pTypeInfo->category).ToStringView();
        type += '.';
        type += pTypeInfo->detailedType.ToStringView();

        if (pTypeInfo->detailedSize > 0) {
            type += '(';
            FormatTo(type, "{}", pTypeInfo->detailedSize);
            type += ')';
        }

        if (pTypeInfo->pNext[0] || pTypeInfo->pNext[1]) {
            type += '[';

            for (uint32_t i = 0; i < 2; ++i) {
                if (pTypeInfo->pNext[i]) {
                    type += '[';
                    SaveTypeInfoInternal(type, pTypeInfo->pNext[i]);
                    type += ']';
                }

                if (i == 0 && pTypeInfo->pNext[1]) {
                    type += ',';
                }
            }

            type += ']';
        }
    }

    void SaveTypeInfo(String& type, const TypeInfo* pTypeInfo) {
        type.clear();
        type.reserve(64);
        SaveTypeInfoInternal(type, pTypeInfo);
    }
}