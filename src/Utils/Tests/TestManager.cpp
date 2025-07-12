//
// Created by innerviewer on 2024-03-18.
//

#include <Utils/Tests/TestManager.h>
#include <Utils/Platform/Platform.h>

#include <Codegen/TestManager.generated.hpp>

namespace SR_UTILS_NS {
    ITestController::ITestController()
        : SR_HTYPES_NS::SharedPtr<ITestController>(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
    { }

    TestExecutionResult TestManager::RunAll() {
        SR_TRACY_ZONE;

        auto&& applicationPath = SR_PLATFORM_NS::GetApplicationPath().GetFolder();
        auto&& resourcePath = SR_PLATFORM_NS::GetApplicationResourcesPath();

        if (!SR_UTILS_NS::Debug::Instance().IsInitialized()) {
            SR_UTILS_NS::Debug::Instance().Initialize(applicationPath.Concat("srengine-tests.log"), true, SR_UTILS_NS::Debug::Theme::Dark);
            SR_UTILS_NS::Debug::Instance().SetLevel(SR_UTILS_NS::Debug::Level::Low);
        }

        if (!SR_UTILS_NS::ResourceManager::Instance().IsInitialized()) {
            SR_UTILS_NS::ResourceManager::Instance().Initialize(resourcePath);
        }

        SR_LOG_TEST("TestManager::RunAll() : loading TestManagerAsset...");

        SRAssert(!m_pTestManagerAsset);
        m_pTestManagerAsset = SR_UTILS_NS::Asset::Load<TestManagerAsset>("ModuleTests/TestManagerAsset.sras");
        if (!m_pTestManagerAsset) {
            SR_LOG_TEST("TestManager::RunAll() : TestManagerAsset not found, creating a new one...");
            m_pTestManagerAsset = SR_UTILS_NS::Asset::CreateNew<TestManagerAsset>("ModuleTests/TestManagerAsset.sras");
            if (!m_pTestManagerAsset) {
                SR_LOG_TEST("TestManager::RunAll() : failed to create TestManagerAsset!");
                return TestExecutionResult::Fatal;
            }
        }

        m_pTestManagerAsset->AddUsePoint();

        uint32_t totalTests = 0;
        for (auto&& pTest : m_pTestManagerAsset->m_tests) {
            if (pTest) {
                totalTests += pTest->GetTotalTestsCount();
            }
        }

        if (totalTests == 0) {
            SR_LOG_TEST("TestManager::RunAll() : no tests found to run!");
            m_pTestManagerAsset->RemoveUsePoint();
            m_pTestManagerAsset.Reset();
            return TestExecutionResult::Success;
        }

        SR_LOG_TEST("TestManager::RunAll() : running {} tests...", totalTests);

        m_testResults.fill(0);

        for (auto&& pTest : m_pTestManagerAsset->m_tests) {
            if (!pTest) {
                SR_LOG_TEST("TestManager::RunAll() : test is null, skipping...");
                OnTestResult(TestExecutionResult::Skipped);
                continue;
            }

            SR_LOG_TEST("TestManager::RunAll() : running test '{}'.", pTest->GetMeta()->GetFactoryName());

            OnTestResult(pTest->Run());

            if (HasFatalError()) {
                SR_LOG_TEST("TestManager::RunAll() : test '{}' finished with fatal error!", pTest->GetMeta()->GetFactoryName());
                m_pTestManagerAsset->RemoveUsePoint();
                m_pTestManagerAsset.Reset();
                return TestExecutionResult::Fatal;
            }
        }

        m_pTestManagerAsset->RemoveUsePoint();
        m_pTestManagerAsset.Reset();

        SR_LOG_TEST("TestManager::RunAll() : all tests finished! Test results:"
                    "\n\tSuccess: {}\n\tError: {}\n\tSkipped: {}",
                    m_testResults[static_cast<uint32_t>(TestExecutionResult::Success)],
                    m_testResults[static_cast<uint32_t>(TestExecutionResult::Error)],
                    m_testResults[static_cast<uint32_t>(TestExecutionResult::Skipped)]);

        if (m_testResults[static_cast<uint32_t>(TestExecutionResult::Error)] > 0) {
            SR_LOG_TEST("TestManager::RunAll() : some tests finished with errors!");
            return TestExecutionResult::Error;
        }

        return TestExecutionResult::Success;
    }

    void TestManager::OnTestResult(TestExecutionResult result) {
        if (result == TestExecutionResult::Fatal) {
            m_hasFatalError = true;
        }
        m_testResults[static_cast<uint32_t>(result)]++;
    }

    uint32_t TestGroupController::GetTotalTestsCount() const {
        uint32_t count = 0;
        for (auto&& pTest : m_tests) {
            if (pTest) {
                count += pTest->GetTotalTestsCount();
            }
        }
        return count;
    }

    TestExecutionResult TestGroupController::Run() {
        for (auto&& pTest : m_tests) {
            if (!pTest) {
                SR_LOG_TEST("TestGroupController::Run() : test is null, skipping...");
                TestManager::Instance().OnTestResult(TestExecutionResult::Skipped);
                continue;
            }

            SR_LOG_TEST("TestGroupController::Run() : running test '{}'.", pTest->GetMeta()->GetFactoryName());

            TestManager::Instance().OnTestResult(pTest->Run());

            if (TestManager::Instance().HasFatalError()) {
                SR_LOG_TEST("TestGroupController::Run() : test '{}' finished with fatal error!", pTest->GetMeta()->GetFactoryName());
                return TestExecutionResult::Fatal;
            }
        }

        return TestExecutionResult::Success;
    }
}
