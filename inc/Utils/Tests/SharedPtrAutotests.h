//
// Created by Monika on 18.03.2024.
//

#ifndef SR_ENGINE_SHARED_PTR_AUTO_TESTS_H
#define SR_ENGINE_SHARED_PTR_AUTO_TESTS_H

#include <Utils/Tests/TestManager.h>

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

    class SharedPtrTest : public SR_UTILS_NS::ITestController {
        SR_CLASS()
    public:
        SR_UTILS_NS::TestExecutionResult Run() override {
            SR_TRACY_ZONE;

            auto&& pointersCountBefore = SR_HTYPES_NS::SharedPtrDynamicDataCounter::Instance().GetCount();

            {
                using TestClass = AutoTests::ManuallySharedPtrTestClass;
                SR_HTYPES_NS::SharedPtr<TestClass> pInt = new TestClass();
                pInt.AutoFree();
                pInt = new TestClass();
                pInt.AutoFree();
            }

            {
                using TestClass = AutoTests::ManuallySharedPtrTestClassInherit;
                SR_HTYPES_NS::SharedPtr<TestClass> pInt2;
                {
                    SR_HTYPES_NS::SharedPtr<AutoTests::ManuallySharedPtrTestClass> pInt = new TestClass();
                    pInt.AutoFree();
                    pInt2 = SR_UTILS_NS::DynamicPointerCast<TestClass>(pInt);
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

                for (auto &pInt2: v) {
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
                auto &&fn = [pIntCopy = pInt.GetThis()]() {};
                SR_HTYPES_NS::Function<void()> fn1 = [pIntCopy = pInt.GetThis()]() {};
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

            {
                std::vector<SR_HTYPES_NS::SharedPtr<AutoTests::AutomaticallySharedPtrTestClass>> v;
                v.emplace_back(new AutoTests::AutomaticallySharedPtrTestClass());
                v.emplace_back(new AutoTests::AutomaticallySharedPtrTestClass());
                for (auto&& pInt : v) {
                    pInt.AutoFree();
                }
                auto newVector = std::vector<SR_HTYPES_NS::SharedPtr<AutoTests::AutomaticallySharedPtrTestClass>>({ new AutoTests::AutomaticallySharedPtrTestClass() });
                v = newVector;
                v.emplace_back(new AutoTests::AutomaticallySharedPtrTestClass());
            }

            auto&& pointersCountCurrent = SR_HTYPES_NS::SharedPtrDynamicDataCounter::Instance().GetCount();
            if (pointersCountCurrent != pointersCountBefore) {
                SR_ERROR("SharedPtrTest::Run() : pointers count before: {}, current: {}", pointersCountBefore, pointersCountCurrent);
                if (pointersCountCurrent > pointersCountBefore) {
                    SR_ERROR("SharedPtrTest::Run() : pointers count increased, memory leak detected!");
                }
                else {
                    SR_ERROR("SharedPtrTest::Run() : pointers count decreased, memory corruption detected!");
                }
                for (auto&& pData : SR_HTYPES_NS::SharedPtrDynamicDataCounter::Instance().GetData()) {
                    SR_ERROR("SharedPtrTest::Run() : pointer with strong {} trace: {}", pData->GetStrongCount(), pData->GetDebugTrace().ToCStr());
                }
                return SR_UTILS_NS::TestExecutionResult::Fatal;
            }

            return SR_UTILS_NS::TestExecutionResult::Success;
        }
    };
}

#endif //SR_ENGINE_SHARED_PTR_AUTO_TESTS_H
