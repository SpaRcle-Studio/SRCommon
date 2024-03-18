//
// Created by Monika on 18.03.2024.
//

#ifndef SR_ENGINE_SHARED_PTR_AUTO_TESTS_H
#define SR_ENGINE_SHARED_PTR_AUTO_TESTS_H

#include <Utils/Types/SharedPtr.h>

namespace SR_UTILS_NS {
    namespace AutoTests {
        class ManuallySharedPtrTestClass : public SR_HTYPES_NS::SharedPtr<ManuallySharedPtrTestClass> {
        public:
            ManuallySharedPtrTestClass()
                : SR_HTYPES_NS::SharedPtr<ManuallySharedPtrTestClass>(this, SR_UTILS_NS::SharedPtrPolicy::Manually)
            { }

            virtual ~ManuallySharedPtrTestClass() = default;

            void DoSomething() {
                m_data.push_back(5);
            }

        private:
            std::vector<int> m_data;

        };

        class ManuallySharedPtrTestClassInherit : public ManuallySharedPtrTestClass {
        public:
            ~ManuallySharedPtrTestClassInherit() override = default;
        };

        class AutomaticallySharedPtrTestClass : public SR_HTYPES_NS::SharedPtr<AutomaticallySharedPtrTestClass> {
        public:
            AutomaticallySharedPtrTestClass()
                : SR_HTYPES_NS::SharedPtr<AutomaticallySharedPtrTestClass>(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
            { }
        };
    }

    static bool RunTestSharedPtr() {
        if (!SR_HTYPES_NS::SharedPtrDynamicDataCounter::CheckMemoryLeaks()) {
            SR_PLATFORM_NS::WriteConsoleError("Memory leaks detected before tests!\n");
            return false;
        }

        {
            using TestClass = AutoTests::ManuallySharedPtrTestClass;
            SR_HTYPES_NS::SharedPtr<TestClass> pInt = new TestClass();
            pInt->AutoFree();
        }

        {
            using TestClass = AutoTests::ManuallySharedPtrTestClassInherit;
            SR_HTYPES_NS::SharedPtr<TestClass> pInt2;
            {
                SR_HTYPES_NS::SharedPtr<AutoTests::ManuallySharedPtrTestClass> pInt = new TestClass();
                pInt->AutoFree();
                pInt2 = pInt.DynamicCast<TestClass>();
            }
            if (pInt2) {
                pInt2->DoSomething();
            }
        }

        {
            using TestClass = AutoTests::ManuallySharedPtrTestClass;

            SR_HTYPES_NS::SharedPtr<TestClass> pInt = TestClass::MakeShared();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt2 = pInt;

            pInt2.AutoFree();

            SR_HTYPES_NS::SharedPtr<TestClass> pInt3 = pInt;
        }

        {
            using TestClass = AutoTests::ManuallySharedPtrTestClass;

            SR_HTYPES_NS::SharedPtr<TestClass> pInt = TestClass::MakeShared();
            std::vector<SR_HTYPES_NS::SharedPtr<TestClass>> v;

            v.emplace_back(pInt);

            for (auto& pInt2 : v) {
                pInt2->DoSomething();
            }

            {
                auto pInt2 = pInt;
                pInt2.AutoFree();
            }

            v = std::vector<SR_HTYPES_NS::SharedPtr<TestClass>>();

            if (pInt) {
                pInt->DoSomething();
            }
        }

        {
            using TestClass = AutoTests::ManuallySharedPtrTestClass;

            SR_HTYPES_NS::SharedPtr<TestClass> pInt = TestClass::MakeShared();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt2 = pInt;
            SR_HTYPES_NS::SharedPtr<TestClass> pInt3 = pInt2.Get();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt4 = pInt2.GetThis();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt5 = std::move(pInt2);

            pInt5.AutoFree();
        }

        {
            using TestClass = AutoTests::ManuallySharedPtrTestClass;

            SR_HTYPES_NS::SharedPtr<TestClass> pInt = new TestClass();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt2 = pInt;
            SR_HTYPES_NS::SharedPtr<TestClass> pInt3 = pInt2.Get();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt4 = pInt2.GetThis();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt5 = std::move(pInt2);

            pInt5.AutoFree();
        }

        {
            using TestClass = AutoTests::ManuallySharedPtrTestClass;

            SR_HTYPES_NS::SharedPtr<TestClass> pInt = new TestClass();
            struct P {
                SR_HTYPES_NS::SharedPtr<TestClass> pInt;
                ~P() {
                    pInt.AutoFree();
                }
            } p;

            p.pInt = pInt;
            pInt = nullptr;
        }

        {
            using TestClass = AutoTests::AutomaticallySharedPtrTestClass;

            SR_HTYPES_NS::SharedPtr<TestClass> pInt = new TestClass();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt2 = pInt;
            SR_HTYPES_NS::SharedPtr<TestClass> pInt3 = pInt2.Get();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt4 = pInt2.GetThis();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt5 = std::move(pInt2);

            pInt5.AutoFree();
        }

        {
            using TestClass = AutoTests::AutomaticallySharedPtrTestClass;

            SR_HTYPES_NS::SharedPtr<TestClass> pInt = new TestClass();
        }

        {
            using TestClass = AutoTests::AutomaticallySharedPtrTestClass;

            SR_HTYPES_NS::SharedPtr<TestClass> pInt = new TestClass();
            auto&& fn = [pIntCopy = pInt.GetThis()]() { };
            SR_HTYPES_NS::Function<void()> fn1 = [pIntCopy = pInt.GetThis()]() { };
        }

        {
            using TestClass = AutoTests::AutomaticallySharedPtrTestClass;

            SR_HTYPES_NS::SharedPtr<TestClass> pInt = new TestClass();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt2 = pInt;
            SR_HTYPES_NS::SharedPtr<TestClass> pInt3 = pInt2.Get();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt4 = pInt2.GetThis();
            SR_HTYPES_NS::SharedPtr<TestClass> pInt5 = std::move(pInt2);
        }

        {
            SR_HTYPES_NS::SharedPtr<int> pInt = new int(5);
        }

        {
            SR_HTYPES_NS::SharedPtr<int> pInt = new int(5);
            pInt.AutoFree();
        }

        {
            SR_HTYPES_NS::SharedPtr<int> pInt = new int(5);
            SR_HTYPES_NS::SharedPtr<int> pInt2 = pInt;
            /// crash SR_HTYPES_NS::SharedPtr<int> pInt3 = pInt2.Get();
            SR_HTYPES_NS::SharedPtr<int> pInt4 = pInt2.GetThis();
            SR_HTYPES_NS::SharedPtr<int> pInt5 = std::move(pInt2);
        }

        {
            SR_HTYPES_NS::SharedPtr<int> pInt = new int(5);
            SR_HTYPES_NS::SharedPtr<int> pInt2 = pInt;
            /// crash SR_HTYPES_NS::SharedPtr<int> pInt3 = pInt2.Get();
            SR_HTYPES_NS::SharedPtr<int> pInt4 = pInt2.GetThis();
            SR_HTYPES_NS::SharedPtr<int> pInt5 = std::move(pInt2);
            pInt5.AutoFree();
        }

        if (!SR_HTYPES_NS::SharedPtrDynamicDataCounter::CheckMemoryLeaks()) {
            return false;
        }

        return true;
    }
}

#endif //SR_ENGINE_SHARED_PTR_AUTO_TESTS_H
