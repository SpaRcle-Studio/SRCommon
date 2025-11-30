//
// Created by Monika on 19.01.2025.
//

#ifndef SR_ENGINE_UTILS_REFLECTION_TEST_COMPONENT_H
#define SR_ENGINE_UTILS_REFLECTION_TEST_COMPONENT_H

#include <Utils/ECS/Component.h>

namespace SR_UTILS_NS::Tests {
    struct ReflectionTestComponentAssociative : public SR_UTILS_NS::Serializable {
        SR_STRUCT()

        /// @property
        std::map<SR_UTILS_NS::StringAtom, int> associativeMapAtom;
        /// @property
        std::map<std::string, int> associativeMapString;
        /// @property
        std::map<int, std::string> associativeMap2;
        /// @property
        std::map<SR_UTILS_NS::PlatformType, int> associativeMap33;
        /// @property
        std::map<SR_UTILS_NS::PlatformType, std::string> associativeMap3;
        /// @property
        std::map<SR_UTILS_NS::PlatformType, SR_UTILS_NS::Path> associativeMap4;

    };

    struct ReflectionTestComponentDataBase : public SR_UTILS_NS::Serializable {
        SR_STRUCT()

        /// @property
        bool baseField = false;

    };

    struct ReflectionTestComponentData : public ReflectionTestComponentDataBase {
        SR_STRUCT()

        /// @property
        bool testField = false;
        /// @property
        SR_MATH_NS::FVector3 vector;
        /// @property
        int32_t m_width = 100;
        /// @property
        std::vector<float> m_test;
    };

    class ReflectionTestComponentLogicBase : public SR_UTILS_NS::Serializable, public SR_HTYPES_NS::SharedPtr<ReflectionTestComponentLogicBase> {
        SR_CLASS()
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<ReflectionTestComponentLogicBase>;

        ReflectionTestComponentLogicBase()
            : SR_HTYPES_NS::SharedPtr<ReflectionTestComponentLogicBase>(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
        { }

    private:
        /// @property
        bool m_baseBoolValue = false;

    };

    class ReflectionTestComponentLogicSimple : public ReflectionTestComponentLogicBase {
        SR_CLASS()
    private:
        /// @property
        float_t m_simpleFloat = 0.0f;
        /// @property
        int32_t m_simpleInt = 0;
    };

    class ReflectionTestComponentLogicComplex : public ReflectionTestComponentLogicBase {
        SR_CLASS()
    private:
        /// @property
        std::vector<int> m_complexInts;
        /// @property
        SR_MATH_NS::FVector3 m_complexVector;
        /// @property
        ReflectionTestComponentLogicBase::Ptr m_other;
        /// @property
        std::vector<ReflectionTestComponentLogicBase::Ptr> m_others;

    };

    /// @category(Debug)
    class ReflectionTestComponent : public SR_UTILS_NS::Component {
        using Super = Component;
        SR_CLASS()
    public:
        SR_NODISCARD const bool& IsWidthChangeable() const { return m_isWidthChangeable; }
        std::vector<float>& GetTest() { return m_test; }

        void OnChanged() {
            m_hasChanged = true;
        }

    private:
        /// @property @onChanged(OnChanged)
        ReflectionTestComponentAssociative m_associative;
        /// @property @onChanged(OnChanged)
        std::string m_string = "Hello world!";
        /// @property @onChanged(OnChanged)
        SR_UTILS_NS::PlatformType m_platformType = SR_UTILS_NS::PlatformType::Windows;
        /// @property
        ReflectionTestComponentData m_data;
        /// @property
        ReflectionTestComponentLogicBase::Ptr m_logic;
        /// @property
        std::vector<ReflectionTestComponentData> m_datas;
        /// @property @onChanged(OnChanged) @getter(IsWidthChangeable)
        bool m_isWidthChangeable = true;
        /// @property @onChanged(OnChanged)
        bool m_isHeightChangeable = true;
        /// @property @onChanged(OnChanged)
        int32_t m_width = 100;
        /// @property @onChanged(OnChanged) @readOnly
        int8_t m_int8 = 5;
        /// @property @onChanged(OnChanged) @drag(5)
        uint64_t m_uint64 = 500 + 400;
        /// @property @onChanged(OnChanged)
        float_t m_height = 200.0f;
        /// @property @onChanged(OnChanged) @drag(0.01f) @resetValue(SR_MATH_NS::FVector3(1.0f, 0.0f, 2.0f))
        SR_MATH_NS::FVector3 m_position;
        /// @property @onChanged(OnChanged) @drag(0.01f)
        SR_MATH_NS::FVector6 m_6d;
        /// @property @onChanged(OnChanged)
        SR_MATH_NS::FVector2 m_2d;
        /// @property @onChanged(OnChanged)
        SR_MATH_NS::BVector4 m_4db;
        /// @property @readOnly @dontSave
        bool m_hasChanged = false;
        /// @property @onChanged(OnChanged)
        SR_MATH_NS::FSize m_size;
        /// @property @onChanged(OnChanged)
        SR_MATH_NS::FSize2 m_size2;
        /// @property @getter(GetTest)
        std::vector<float> m_test;
        /// @property
        std::vector<std::vector<int>> m_test2;
        /// @property
        std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<float_t>>>>>> m_test4;
        /// @property
        std::vector<int> m_test3 = { 1, 2, 3 };
        /// @property
        std::vector<SR_MATH_NS::FVector3> m_testMathVector3;
        /// @property
        std::vector<std::vector<SR_MATH_NS::FVector4>> m_testMathVector4;
        /// @property
        std::vector<SR_UTILS_NS::PlatformType> m_testPlatformType;
        /// @property
        std::vector<SR_MATH_NS::FSize> m_testSize;
        /// @property
        std::vector<SR_MATH_NS::FSize2> m_testSize2;
        /// @property
        std::vector<bool> m_bitSet;

    };
}

#endif //SR_ENGINE_UTILS_REFLECTION_TEST_COMPONENT_H
