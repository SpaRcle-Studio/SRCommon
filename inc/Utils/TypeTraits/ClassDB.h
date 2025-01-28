//
// Created by Monika on 09.10.2024.
//

#ifndef SR_COMMON_CLASS_DB_H
#define SR_COMMON_CLASS_DB_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/Types/StringAtom.h>
#include <Utils/Types/Mutex.h>

namespace SR_UTILS_NS {
    class SRClass;

    struct ClassInfo {
        StringAtom name;
        StringAtom inherits;
        ClassInfo* pInherits = nullptr;
    };

    class ClassDB : public NonCopyable, public NonMovable {
    private:
        ClassDB() = default;
        ~ClassDB() override = default;

    public:
        static ClassDB& Instance();

        void ResolveInheritance();

        /// RegisterClass conflicts with fucking WinAPI macroses
        bool RegisterNewClass(StringAtom className);
        bool RegisterInheritance(StringAtom className, StringAtom inherits);
        bool RegisterProperty(StringAtom className, StringAtom propertyType, StringAtom propertyName);

    private:
        bool m_inheritanceResolved = false;
        std::shared_mutex m_mutex;
        std::unordered_map<StringAtom, ClassInfo> m_classes;
        std::unordered_map<StringAtom, std::vector<StringAtom>> m_inheritance;

    };
}

#endif //SR_COMMON_CLASS_DB_H
