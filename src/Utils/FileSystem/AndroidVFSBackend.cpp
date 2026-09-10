//
// Created by Monika on 10.09.2026.
//

#include <Utils/FileSystem/AndroidVFSBackend.h>
#include <Utils/FileSystem/File.h>
#include <Utils/Platform/Platform.h>

#ifdef SR_ANDROID
    #include <Utils/Platform/AndroidNativeAppGlue.h>

    #include <android/native_activity.h>
    #include <android/asset_manager.h>
    #include <jni.h>
#endif

namespace SR_UTILS_NS {
#ifdef SR_ANDROID
    namespace {
        SR_NODISCARD ANativeActivity* GetNativeActivity() {
            auto&& pApp = reinterpret_cast<android_app*>(SR_PLATFORM_NS::GetInstance());
            if (!pApp || !pApp->activity) {
                SRHalt("AndroidVFSBackend : android application instance is not set!");
                return nullptr;
            }
            return pApp->activity;
        }

        SR_NODISCARD AAssetManager* GetAssetManager() {
            auto&& pActivity = GetNativeActivity();
            return pActivity ? pActivity->assetManager : nullptr;
        }

        SR_NODISCARD bool CheckJNIException(JNIEnv* pEnv) {
            if (pEnv->ExceptionCheck()) {
                pEnv->ExceptionClear();
                return true;
            }
            return false;
        }

        /// AAssetDir перечисляет только файлы, поэтому список директорий приходится получать через JNI
        /// (AssetManager.list() возвращает и файлы, и папки). Виртуальная машина может быть не привязана
        /// к текущему потоку, поэтому привязываем её на время работы.
        class JNIScope : public NonCopyable {
        public:
            JNIScope() {
                auto&& pActivity = GetNativeActivity();
                if (!pActivity || !pActivity->vm) {
                    return;
                }

                m_pVM = pActivity->vm;

                const jint status = m_pVM->GetEnv(reinterpret_cast<void**>(&m_pEnv), JNI_VERSION_1_6);
                if (status == JNI_EDETACHED) {
                    if (m_pVM->AttachCurrentThread(&m_pEnv, nullptr) != JNI_OK) {
                        SR_ERROR("AndroidVFSBackend : failed to attach current thread to the JVM!");
                        m_pEnv = nullptr;
                        return;
                    }
                    m_isAttached = true;
                }
                else if (status != JNI_OK) {
                    SR_ERROR("AndroidVFSBackend : failed to get JNI environment! Status: {}", static_cast<int32_t>(status));
                    m_pEnv = nullptr;
                }
            }

            ~JNIScope() override {
                if (m_isAttached && m_pVM) {
                    m_pVM->DetachCurrentThread();
                }
            }

            SR_NODISCARD JNIEnv* Get() const { return m_pEnv; }
            SR_NODISCARD explicit operator bool() const { return m_pEnv != nullptr; }
            JNIEnv* operator->() const { return m_pEnv; }

        private:
            JavaVM* m_pVM = nullptr;
            JNIEnv* m_pEnv = nullptr;
            bool m_isAttached = false;

        };

        /// Возвращает содержимое директории assets. Путь задаётся относительно корня assets,
        /// пустая строка означает сам корень.
        void ListAssetDirectory(StringView directory, Vector<String>& outNames) {
            SR_TRACY_ZONE;

            JNIScope env;
            if (!env) {
                return;
            }

            auto&& pActivity = GetNativeActivity();
            if (!pActivity) {
                return;
            }

            jclass activityClass = env->GetObjectClass(pActivity->clazz);
            if (!activityClass || CheckJNIException(env.Get())) {
                return;
            }

            jmethodID getAssetsMethod = env->GetMethodID(activityClass, "getAssets", "()Landroid/content/res/AssetManager;");
            if (!getAssetsMethod || CheckJNIException(env.Get())) {
                env->DeleteLocalRef(activityClass);
                return;
            }

            jobject assetManager = env->CallObjectMethod(pActivity->clazz, getAssetsMethod);
            if (!assetManager || CheckJNIException(env.Get())) {
                env->DeleteLocalRef(activityClass);
                return;
            }

            jclass assetManagerClass = env->GetObjectClass(assetManager);
            jmethodID listMethod = assetManagerClass
                ? env->GetMethodID(assetManagerClass, "list", "(Ljava/lang/String;)[Ljava/lang/String;")
                : nullptr;

            if (listMethod && !CheckJNIException(env.Get())) {
                /// StringView не гарантирует нуль-терминатор, поэтому копируем в String
                const String directoryString(directory);
                jstring jDirectory = env->NewStringUTF(directoryString.empty() ? "" : directoryString.c_str());

                auto&& names = static_cast<jobjectArray>(env->CallObjectMethod(assetManager, listMethod, jDirectory));
                if (names && !CheckJNIException(env.Get())) {
                    const jsize count = env->GetArrayLength(names);
                    outNames.reserve(outNames.size() + static_cast<uint64_t>(count));

                    for (jsize i = 0; i < count; ++i) {
                        auto&& jName = static_cast<jstring>(env->GetObjectArrayElement(names, i));
                        if (!jName) {
                            continue;
                        }
                        if (const char* pName = env->GetStringUTFChars(jName, nullptr)) {
                            outNames.emplace_back(pName);
                            env->ReleaseStringUTFChars(jName, pName);
                        }
                        env->DeleteLocalRef(jName);
                    }
                }

                if (names) {
                    env->DeleteLocalRef(names);
                }
                env->DeleteLocalRef(jDirectory);
            }

            if (assetManagerClass) {
                env->DeleteLocalRef(assetManagerClass);
            }
            env->DeleteLocalRef(assetManager);
            env->DeleteLocalRef(activityClass);
        }

        SR_NODISCARD bool IsAssetFile(StringView assetPath) {
            auto&& pAssetManager = GetAssetManager();
            if (!pAssetManager || assetPath.empty()) {
                return false;
            }

            const String assetPathString(assetPath);
            if (AAsset* pAsset = AAssetManager_open(pAssetManager, assetPathString.c_str(), AASSET_MODE_UNKNOWN)) {
                AAsset_close(pAsset);
                return true;
            }
            return false;
        }

        SR_NODISCARD bool IsAssetDirectory(StringView assetPath) {
            if (assetPath.empty()) {
                return true; /// корень assets
            }
            Vector<String> names;
            ListAssetDirectory(assetPath, names);
            return !names.empty();
        }
    }
#endif

    void AndroidVFSBackend::ResolveAssetPath(StringView virtualPath, String& outAssetPath) const {
        /// пути внутри assets совпадают с виртуальными, поэтому достаточно нормализовать слеши
        while (virtualPath.starts_with('/')) {
            virtualPath.remove_prefix(1);
        }

        outAssetPath = virtualPath;

        /// AssetManager не переваривает завершающий слеш
        while (!outAssetPath.empty() && outAssetPath.ends_with('/')) {
            outAssetPath.remove_suffix(1);
        }
    }

    void AndroidVFSBackend::ResolveVirtualPath(StringView virtualPath, String& outRealPath) const {
        ResolveAssetPath(virtualPath, outRealPath);
    }

    FSItemType AndroidVFSBackend::GetType(StringView path) const {
    #ifdef SR_ANDROID
        SR_TRACY_ZONE;

        static String assetPathBuffer;
        ResolveAssetPath(path, assetPathBuffer);

        if (assetPathBuffer.empty()) {
            return FSItemType::Folder; /// корень assets
        }

        if (IsAssetFile(assetPathBuffer)) {
            return FSItemType::File;
        }

        if (IsAssetDirectory(assetPathBuffer)) {
            return FSItemType::Folder;
        }

        return FSItemType::Undefined;
    #else
        SRHalt("AndroidVFSBackend::GetType() : backend is available only on android!");
        return FSItemType::Undefined;
    #endif
    }

    File AndroidVFSBackend::OpenFile(StringView path, FileMode mode) const {
    #ifdef SR_ANDROID
        SR_TRACY_ZONE;

        if (SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Write)) {
            SRHalt("AndroidVFSBackend::OpenFile() : assets are read-only!");
            return File();
        }

        auto&& pAssetManager = GetAssetManager();
        if (!pAssetManager) {
            return File();
        }

        static String assetPathBuffer;
        ResolveAssetPath(path, assetPathBuffer);

        if (assetPathBuffer.empty()) {
            SR_ERROR("AndroidVFSBackend::OpenFile() : resolved asset path is empty! Path: {}", path);
            return File();
        }

        AAsset* pAsset = AAssetManager_open(pAssetManager, assetPathBuffer.c_str(), AASSET_MODE_BUFFER);
        if (!pAsset) {
            SR_WARN("AndroidVFSBackend::OpenFile() : asset not found!\n\tPath: {}", assetPathBuffer);
            return File();
        }

        const auto size = static_cast<uint64_t>(AAsset_getLength64(pAsset));

        String data;
        data.resize(static_cast<SizeType>(size));

        if (size > 0) {
            const int readBytes = AAsset_read(pAsset, data.data(), static_cast<size_t>(size));
            if (readBytes < 0 || static_cast<uint64_t>(readBytes) != size) {
                SR_ERROR("AndroidVFSBackend::OpenFile() : failed to read asset!\n\tPath: {}", assetPathBuffer);
                AAsset_close(pAsset);
                return File();
            }
        }

        AAsset_close(pAsset);

        /// assets внутри apk могут быть сжаты, поэтому отобразить их в память нельзя - читаем целиком.
        /// MemoryFileImpl отдает содержимое через Data(), так что FileMode::Map тоже отрабатывает.
        auto&& pImpl = new MemoryFileImpl(std::move(data));
        if (auto&& file = File(pImpl, assetPathBuffer, mode)) {
            return file;
        }
        return File();
    #else
        SRHalt("AndroidVFSBackend::OpenFile() : backend is available only on android!");
        return File();
    #endif
    }

    void AndroidVFSBackend::Enumerate(StringView directory, const IVFSBackend::EnumerateCallback& callback, bool recursive) const {
    #ifdef SR_ANDROID
        SR_TRACY_ZONE;

        String rootAssetPath;
        ResolveAssetPath(directory, rootAssetPath);

        auto&& enumerateDirectory = [&](auto&& self, const String& assetDirectory) -> void {
            Vector<String> names;
            ListAssetDirectory(assetDirectory, names);

            for (auto&& name : names) {
                String assetPath = assetDirectory;
                if (!assetPath.empty()) {
                    assetPath += '/';
                }
                assetPath += name;

                const bool isFile = IsAssetFile(assetPath);
                if (!isFile && !IsAssetDirectory(assetPath)) {
                    continue;
                }

                StringView extension = name;
                if (auto&& dotPos = extension.find_last_of('.'); dotPos != StringView::npos) {
                    extension.remove_prefix(dotPos + 1);
                }
                else {
                    extension = {};
                }

                VFSEntry vfsEntry;
                vfsEntry.name = name;
                vfsEntry.extension = extension;
                /// путь внутри assets и есть виртуальный путь, отдельного реального пути нет
                vfsEntry.fullPath = assetPath;
                vfsEntry.relativePath = assetPath;
                vfsEntry.type = isFile ? FSItemType::File : FSItemType::Folder;

                callback(vfsEntry);

                if (recursive && !isFile) {
                    self(self, assetPath);
                }
            }
        };

        enumerateDirectory(enumerateDirectory, rootAssetPath);
    #else
        SRHalt("AndroidVFSBackend::Enumerate() : backend is available only on android!");
    #endif
    }

    void AndroidVFSBackend::Delete(StringView path) const {
        SRHalt("AndroidVFSBackend::Delete() : not supported!");
    }
}
