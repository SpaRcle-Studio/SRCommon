//
// Created by Monika on 11.08.2026.
//

#include <Utils/Reflection/VTable.h>
#include <Utils/Reflection/ReflectedType.h>
#include <Utils/TypeTraits/Factory.h>
#include <Utils/Common/EnumReflector.h>

namespace SR_UTILS_NS::Reflection {
    template<SizeType Size> void ArithmeticTypeCopy(const ReflectedValue& from, ReflectedValue& to) {
        memcpy(to.GetData(), from.GetData(), Size);
    }

    template<SizeType Size> void ArithmeticTypeMove(ReflectedValue& from, ReflectedValue& to) {
        memcpy(to.GetData(), from.GetData(), Size);
        memset(from.GetData(), 0, Size);
    }

    template<SizeType Size> Pair<SizeType, SizeType> ArithmeticTypeSizeOfAlign(const TypeInfo&) {
        return { Size, Size };
    }

    ReflectedValue ArithmeticTypeConstructor(IAllocator&) {
        return ReflectedValue::MakeFromNumeric(uint64_t(), ReflectedValueStorageType::Embedded);
    }

    void FillArithmeticVTable(TypeInfoVTable& vtable, const SizeType size) {
        vtable.pConstructor = &ArithmeticTypeConstructor;

        switch (size) {
            case 1:
                vtable.pCopy = &ArithmeticTypeCopy<1>;
                vtable.pMove = &ArithmeticTypeMove<1>;
                vtable.pSizeOfAlign = &ArithmeticTypeSizeOfAlign<1>;
                break;
            case 2:
                vtable.pCopy = &ArithmeticTypeCopy<2>;
                vtable.pMove = &ArithmeticTypeMove<2>;
                vtable.pSizeOfAlign = &ArithmeticTypeSizeOfAlign<2>;
                break;
            case 4:
                vtable.pCopy = &ArithmeticTypeCopy<4>;
                vtable.pMove = &ArithmeticTypeMove<4>;
                vtable.pSizeOfAlign = &ArithmeticTypeSizeOfAlign<4>;
                break;
            case 8:
                vtable.pCopy = &ArithmeticTypeCopy<8>;
                vtable.pMove = &ArithmeticTypeMove<8>;
                vtable.pSizeOfAlign = &ArithmeticTypeSizeOfAlign<8>;
                break;
            default:
                SRHalt("FillArithmeticVTable() : unsupported arithmetic size {}!", size);
                break;
        }
    }

    bool ResolveBuiltinVTable(const TypeInfoKey& key, TypeInfoVTable& vtable) {
        if (key.kind == TypeKind::Enum) {
            auto&& pReflector = EnumReflectorManager::Instance().TryGetReflector(key.name);
            if (!pReflector) {
                return false;
            }
            FillArithmeticVTable(vtable, pReflector->GetIntegralTypeSizeInternal());
            return true;
        }

        if (key.kind == TypeKind::Object) {
            if (auto&& pMeta = Factory::Instance().GetType(key.name)) {
                vtable = pMeta->GetVTable();
                return true;
            }
            return false;
        }

        return false;
    }
}
