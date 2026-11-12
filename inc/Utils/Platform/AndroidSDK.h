#ifndef SR_UTILS_ANDROID_SDK_H
#define SR_UTILS_ANDROID_SDK_H

#include <Utils/Platform/Process.h>
#include <Utils/Types/Function.h>

namespace SR_UTILS_NS::Platform {
    class SR_COMMON_DLL_API AndroidSDK : public NonCopyable {
    public:
        struct Device {
            String serial;
            String state;
            String description;
        };

        struct SystemImage {
            String package;
            String description;
            bool installed = false;
        };

        explicit AndroidSDK(Path root);
        Path GetSDKPath() const;
        Path GetHomePath() const;
        Path GetJavaPath(StringView tool) const;
        Path GetADBPath() const;
        Path GetLogPath() const;
        String GetSelectedImage() const;
        bool HasTools() const;
        bool HasEmulator(StringView image) const;
        bool IsSystemImageInstalled(StringView image) const;

        bool Install(bool withEmulator, bool acceptedLicense, StringView image);
        bool ListSystemImages(Vector<SystemImage>& images);
        bool GetDevices(Vector<Device>& devices);
        bool StartEmulator(Process& process, StringView image);
        bool InstallAndRun(StringView serial, const Path& arm64Apk, const Path& x64Apk);
        bool Sign(const Path& apk, const Path& keyStore);
        bool CheckPackage(const Path& apk, StringView package);
        bool ReadLogcat(StringView serial, StringView package, StringView priority, StringView buffer, String& output);
        bool ClearLogcat(StringView serial);
        const String& GetLastPackage() const { return m_lastPackage; }

        SR_HTYPES_NS::Function<bool()> isCancelled;
        SR_HTYPES_NS::Function<void(String, bool)> onMessage;
        SR_HTYPES_NS::Function<void(String)> onCommandOutput;
        SR_HTYPES_NS::Function<void(float_t)> onProgress;
        SR_HTYPES_NS::Function<void(uint64_t, uint64_t)> onDownloadProgress;

    private:
        void Message(String text, bool error = false) const;
        bool Command(const Path& program, const Vector<String>& arguments, String* output = nullptr,
            uint32_t timeoutSeconds = 120, StringView input = {}, bool quiet = false);
        bool JavaTool(StringView tool, const Vector<String>& arguments, uint32_t timeout = 1800, StringView input = {});
        bool Download(StringView url, const Path& destination);
        String ReadCommandOutput(const Path& path) const;
        Vector<String> Environment() const;
        String GetVirtualDeviceName(StringView image) const;
        Path GetVirtualDevicePath(StringView image) const;
        Path m_root;
        String m_lastPackage;
    };
}

#endif
