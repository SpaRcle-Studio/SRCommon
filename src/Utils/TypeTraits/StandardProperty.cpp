//
// Created by Monika on 07.11.2023.
//

#include <Utils/TypeTraits/StandardProperty.h>

namespace SR_UTILS_NS {
    /// ---------------------------------------- StandardProperty ------------------------------------------------------

    void StandardProperty::SaveProperty(MarshalRef marshal) const noexcept {
        if (auto&& pBlock = AllocatePropertyBlock()) {
            pBlock->Write<StringAtom>(SR_UTILS_NS::EnumReflector::ToStringAtom(GetStandardType()));

            switch (GetStandardType()) {
                case StandardType::Bool: pBlock->Write<bool>(GetBool()); break;
                case StandardType::Float: pBlock->Write<float_t>(GetFloat()); break;
                case StandardType::Int16: pBlock->Write<int32_t>(GetInt16()); break;
                case StandardType::UInt16: pBlock->Write<uint32_t>(GetUInt16()); break;
                case StandardType::Int32: pBlock->Write<int32_t>(GetInt32()); break;
                case StandardType::UInt32: pBlock->Write<uint32_t>(GetUInt32()); break;
                case StandardType::String: pBlock->Write<std::string>(GetString()); break;
                case StandardType::UnicodeString: pBlock->Write<SR_HTYPES_NS::UnicodeString>(GetUnicodeString()); break;
                case StandardType::StringAtom: pBlock->Write<StringAtom>(GetStringAtom()); break;
                case StandardType::FVector2: pBlock->Write<SR_MATH_NS::FVector2>(GetFVector2()); break;
                case StandardType::UVector2: pBlock->Write<SR_MATH_NS::UVector2>(GetUVector2()); break;
                case StandardType::FVector3: pBlock->Write<SR_MATH_NS::FVector3>(GetFVector3()); break;
                case StandardType::FVector4: pBlock->Write<SR_MATH_NS::FVector4>(GetFVector4()); break;
                case StandardType::BVector3: pBlock->Write<SR_MATH_NS::BVector3>(GetBVector3()); break;
                default:
                    SRHalt("Unsupported type! Type: \"" + SR_UTILS_NS::EnumReflector::ToStringAtom(GetStandardType()).ToStringRef() + "\"");
                    return;
            }

            SavePropertyBase(marshal, std::move(pBlock));
        }
    }

    void StandardProperty::LoadProperty(MarshalRef marshal) noexcept {
        if (auto&& pBlock = LoadPropertyBase(marshal)) {
            auto&& standardType = SR_UTILS_NS::EnumReflector::FromString<StandardType>(pBlock->Read<StringAtom>());
            if (standardType != GetStandardType()) {
                SR_WARN("StandardProperty::LoadProperty() : incompatible properties!\n\tName: {}\n\tProperty type: {}\n\tLoaded type: {}",
                      GetName().ToCStr(),
                      SR_UTILS_NS::EnumReflector::ToStringAtom(GetStandardType()).ToCStr(),
                      SR_UTILS_NS::EnumReflector::ToStringAtom(standardType).ToCStr()
                );
                return;
            }

            switch (GetStandardType()) {
                case StandardType::Bool: SetBool(pBlock->Read<bool>()); break;
                case StandardType::Float: SetFloat(pBlock->Read<float_t>()); break;
                case StandardType::Int16: SetInt16(pBlock->Read<int16_t>()); break;
                case StandardType::UInt16: SetUInt16(pBlock->Read<uint16_t>()); break;
                case StandardType::Int32: SetInt32(pBlock->Read<int32_t>()); break;
                case StandardType::UInt32: SetUInt32(pBlock->Read<uint32_t>()); break;
                case StandardType::String: SetString(pBlock->Read<std::string>()); break;
                case StandardType::UnicodeString: SetUnicodeString(pBlock->Read<SR_HTYPES_NS::UnicodeString>()); break;
                case StandardType::StringAtom: SetStringAtom(pBlock->Read<StringAtom>()); break;
                case StandardType::UVector2: SetUVector2(pBlock->Read<SR_MATH_NS::UVector2>()); break;
                case StandardType::FVector2: SetFVector2(pBlock->Read<SR_MATH_NS::FVector2>()); break;
                case StandardType::FVector3: SetFVector3(pBlock->Read<SR_MATH_NS::FVector3>()); break;
                case StandardType::FVector4: SetFVector4(pBlock->Read<SR_MATH_NS::FVector4>()); break;
                case StandardType::BVector3: SetBVector3(pBlock->Read<SR_MATH_NS::BVector3>()); break;
                default:
                    SRHalt("Unsupported type! Type: \"" + SR_UTILS_NS::EnumReflector::ToStringAtom(GetStandardType()).ToStringRef() + "\"");
                    return;
            }
        }
    }

    /// ------------------------------------------ PathProperty --------------------------------------------------------

    PathProperty& PathProperty::SetPath(const Path& path) noexcept {
        if (m_setter) {
            m_setter(path);
        }
        return *this;
    }

    SR_UTILS_NS::Path PathProperty::GetPath() const noexcept {
        return m_getter ? m_getter() : SR_UTILS_NS::Path();
    }

    void PathProperty::SaveProperty(MarshalRef marshal) const noexcept {
        if (auto&& pBlock = AllocatePropertyBlock()) {
            pBlock->Write<std::string>(GetPath().ToStringRef());
            SavePropertyBase(marshal, std::move(pBlock));
        }
    }

    void PathProperty::LoadProperty(MarshalRef marshal) noexcept {
        if (auto&& pBlock = LoadPropertyBase(marshal)) {
            SetPath(pBlock->Read<std::string>());
        }
    }

    /// ------------------------------------------ EnumProperty --------------------------------------------------------

    SR_UTILS_NS::StringAtom EnumProperty::GetEnum() const noexcept {
        return m_getter ? m_getter() : SR_UTILS_NS::StringAtom();
    }

    EnumProperty& EnumProperty::SetEnum(const StringAtom& value) noexcept {
        if (m_setter) {
            m_setter(value);
        }
        return *this;
    }

    void EnumProperty::SaveProperty(MarshalRef marshal) const noexcept {
        if (auto&& pBlock = AllocatePropertyBlock()) {
            pBlock->Write<StringAtom>(GetEnum());
            SavePropertyBase(marshal, std::move(pBlock));
        }
    }

    void EnumProperty::LoadProperty(MarshalRef marshal) noexcept {
        if (auto&& pBlock = LoadPropertyBase(marshal)) {
            SetEnum(pBlock->Read<StringAtom>());
        }
    }

    void ArrayReferenceProperty::SaveProperty(SR_HTYPES_NS::Marshal& marshal) const noexcept {
        if (auto&& pBlock = AllocatePropertyBlock()) {
            if (m_saveArray) {
                m_saveArray(*pBlock);
            }
            SavePropertyBase(marshal, std::move(pBlock));
        }
        Super::SaveProperty(marshal);
    }

    void ArrayReferenceProperty::LoadProperty(SR_HTYPES_NS::Marshal& marshal) noexcept {
        if (auto&& pBlock = LoadPropertyBase(marshal)) {
            if (m_loadArray) {
                m_loadArray(*pBlock);
            }
        }
    }
}