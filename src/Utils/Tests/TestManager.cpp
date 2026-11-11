//
// Created by innerviewer on 2024-03-18.
//

#include <Utils/Tests/TestManager.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Debug.h>
#include <Utils/FileSystem/VFS.h>
#include <Utils/FileSystem/DirectoryVFSBackend.h>
#include <Utils/FileSystem/ArchiveVFSBackend.h>
#include <Utils/Common/StoreUtils.h>

#include <Codegen/TestManager.generated.hpp>

namespace SR_UTILS_NS {
    ITestController::ITestController()
        : SR_HTYPES_NS::SharedPtr<ITestController>(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
    { }

    TestExecutionResult TestManager::RunAll() {
        SR_TRACY_ZONE;

        auto&& applicationPath = SR_PLATFORM_NS::GetApplicationPath().GetFolder();

        if (!SR_UTILS_NS::Debug::Instance().IsInitialized()) {
            SR_UTILS_NS::Debug::Instance().Initialize(applicationPath.Concat("srengine-tests.log"), true, SR_UTILS_NS::Debug::Theme::Dark);
            SR_UTILS_NS::Debug::Instance().SetLevel(SR_UTILS_NS::Debug::Level::Low);
        }

        if (auto&& appFolder = SR_PLATFORM_NS::GetApplicationDirectory(); !appFolder.empty()) {
            SR_UTILS_NS::VFS::Instance().Mount("", new SR_UTILS_NS::ReadOnlyDirectoryVFSBackend(appFolder), -50);
        }

        bool embeddedResources = false;
    #if defined(SR_WIN32) || defined(SR_LINUX)
        auto* pArchive = new ArchiveVFSBackend(SR_PLATFORM_NS::GetApplicationPath().ToStringView(), ArchiveVFSBackendMode::File);
        if (pArchive->IsValid()) {
            VFS::Instance().Mount("", pArchive, 0);
            embeddedResources = true;
        }
        else {
            delete pArchive;
        }
    #endif
        StoreUtils::Temp::SetBool("ApplicationHasEmbeddedResources", embeddedResources);
        if (!embeddedResources) {
            const auto engineResourcesPath = SR_PLATFORM_NS::GetApplicationResourcesPath();
            const auto resourcesPath = ResourceManager::Instance().GetResPath();
            VFS::Instance().Mount(resourcesPath, new DirectoryVFSBackend(engineResourcesPath), 0);
        }
        const auto userDataPath = SR_PLATFORM_NS::GetApplicationUserDataDirectory();
        VFS::Instance().Mount(userDataPath, new DirectoryVFSBackend(userDataPath), 0);

        if (!SR_UTILS_NS::ResourceManager::Instance().IsInitialized()) {
            SR_UTILS_NS::ResourceManager::Instance().Initialize();
        }

        if (!SR_UTILS_NS::ResourceManager::Instance().InitCache()) {
            SR_ERROR("TestManager::RunAll() : failed to initialize cache!");
            return TestExecutionResult::Fatal;
        }

        SR_LOG_TEST("TestManager::RunAll() : loading TestManagerAsset...");

        SRAssert(!m_pTestManagerAsset);
        m_pTestManagerAsset = SR_UTILS_NS::Asset::Load<TestManagerAsset>("Engine/ModuleTests/TestManagerAsset.sras");
        if (!m_pTestManagerAsset) {
            if (embeddedResources) {
                SR_ERROR("TestManager::RunAll() : embedded TestManagerAsset not found!");
                return TestExecutionResult::Fatal;
            }
            SR_LOG_TEST("TestManager::RunAll() : TestManagerAsset not found, creating a new one...");
            m_pTestManagerAsset = SR_UTILS_NS::Asset::CreateNew<TestManagerAsset>("Engine/ModuleTests/TestManagerAsset.sras");
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
            return embeddedResources ? TestExecutionResult::Fatal : TestExecutionResult::Success;
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
