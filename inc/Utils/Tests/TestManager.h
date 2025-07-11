//
// Created by innerviewer on 2024-03-18.
//

#ifndef SR_ENGINE_COMMON_TESTS_TEST_MANAGER_H
#define SR_ENGINE_COMMON_TESTS_TEST_MANAGER_H

#include <Utils/Common/Singleton.h>
#include <Utils/Common/Coroutine.h>
#include <Utils/Common/Enumerations.h>
#include <Utils/Serialization/Serializable.h>
#include <Utils/Types/SharedPtr.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(TestExecutionResult, uint8_t,
        Success,
        Error,
        Fatal,
        Skipped
    );

    class ITestController : public SR_UTILS_NS::Serializable, public SR_HTYPES_NS::SharedPtr<ITestController> {
        SR_CLASS()
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<ITestController>;
        using OriginType = ITestController;

    public:
        virtual TestExecutionResult Run() = 0;
        SR_NODISCARD virtual uint32_t GetTotalTestsCount() const { return 1; }

    };

    class TestManager : public Singleton<TestManager> {
        SR_REGISTER_SINGLETON(TestManager)
    public:
        void RunAll();

        bool IsSingletonCanBeDestroyed() const override { return false; }

    private:
        ITestController::Ptr m_pTestController = nullptr;

    };
}

#endif //SR_ENGINE_COMMON_TESTS_TEST_MANAGER_H
