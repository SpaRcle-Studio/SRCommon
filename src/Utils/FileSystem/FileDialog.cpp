//
// Created by Monika on 19.06.2022.
//

#include <Utils/FileSystem/FileDialog.h>
#include <Utils/FileSystem/VFS.h>

#if !defined(SR_ANDROID) && !defined(SR_EMSCRIPTEN)
    #include <nfd.h>
#endif

#if defined(SR_EMSCRIPTEN)
    #include <emscripten/emscripten.h>
    EM_ASYNC_JS(char*, SRWebPickFile, (int directory, const char* accept), {
        const path = await Module['srChooseFile'](!!directory, UTF8ToString(accept));
        if (!path) return 0;
        const size = lengthBytesUTF8(path) + 1;
        const result = _malloc(size);
        stringToUTF8(path, result, size);
        return result;
    });
    EM_JS(char*, SRWebSavePath, (const char* initial), {
        const path = Module['srSavePath'](UTF8ToString(initial));
        if (!path) return 0;
        const size = lengthBytesUTF8(path) + 1;
        const result = _malloc(size);
        stringToUTF8(path, result, size);
        return result;
    });
    EM_JS_DEPS(srengine_file_dialog, "$UTF8ToString,$lengthBytesUTF8,$stringToUTF8,malloc");
#endif

namespace SR_UTILS_NS {
    void FileDialog::InitSingleton() {
        Singleton::InitSingleton();
    #if !defined(SR_ANDROID) && !defined(SR_EMSCRIPTEN)
        NFD_Init();
    #endif
    }

    void FileDialog::OnSingletonDestroy() {
    #if !defined(SR_ANDROID) && !defined(SR_EMSCRIPTEN)
        NFD_Quit();
    #endif
        Singleton::OnSingletonDestroy();
    }

    Path SR_UTILS_NS::FileDialog::PickFolder(const Path& defaultPath) const {
        String path;
    #if defined(SR_EMSCRIPTEN)
        if (auto* selected = SRWebPickFile(1, "")) { path = selected; std::free(selected); }
    #endif

    #if !defined(SR_ANDROID) && !defined(SR_EMSCRIPTEN)
        nfdchar_t *outPath;

        SR_UTILS_NS::Path resolvedDefaultPath = defaultPath;
        SR_UTILS_NS::VFS::Instance().ResolvePath(resolvedDefaultPath, FileMode::Write);

        nfdresult_t result = NFD_PickFolder(&outPath, resolvedDefaultPath.c_str());

        if (result == NFD_OKAY)
        {
            path = String(outPath);
            NFD_FreePath(outPath);
        }
        else if (result == NFD_CANCEL)
        {
            /// ignore
        }
        else
        {
            /// ignore
        }
    #endif

        return path;
    }

    Path FileDialog::SaveDialog(const Path &defaultPath, const Filter& filter) const {
        String path;
    #if defined(SR_EMSCRIPTEN)
        if (auto* selected = SRWebSavePath(defaultPath.c_str())) { path = selected; std::free(selected); }
    #endif

    #if !defined(SR_ANDROID) && !defined(SR_EMSCRIPTEN)
        nfdchar_t *outPath;

        nfdfilteritem_t* pFilterItem = filter.size() > 0 ? new nfdfilteritem_t[filter.size()] : nullptr;

        for (uint32_t i = 0; i < static_cast<uint32_t>(filter.size()); ++i) {
            pFilterItem[i].name = filter[i].first.c_str();
            pFilterItem[i].spec = filter[i].second.c_str();
        }

        SR_UTILS_NS::Path resolvedDefaultPath = defaultPath;
        SR_UTILS_NS::VFS::Instance().ResolvePath(resolvedDefaultPath, FileMode::Write);

        nfdresult_t result = NFD_SaveDialog(&outPath, pFilterItem, static_cast<nfdfiltersize_t>(filter.size()), resolvedDefaultPath.c_str(), NULL);
        if (result == NFD_OKAY)
        {
            path = String(outPath);
            NFD_FreePath(outPath);
        }
        else if (result == NFD_CANCEL)
        {
            /// ignore
        }
        else
        {
            /// ignore
        }
    #endif

        return path;
    }

    Path FileDialog::OpenDialog(const Path& defaultPath, const Filter& filter) const {
        String path;
    #if defined(SR_EMSCRIPTEN)
        String accept;
        for (const auto& item : filter) {
            StringView extensions(item.second);
            while (!extensions.empty()) {
                const auto comma = extensions.find(',');
                const auto extension = extensions.substr(0, comma);
                if (!extension.empty() && extension != "*") {
                    if (!accept.empty()) { accept.append(","); }
                    accept.append("."); accept.append(extension);
                }
                if (comma == StringView::npos) { break; }
                extensions.remove_prefix(comma + 1);
            }
        }
        if (auto* selected = SRWebPickFile(0, accept.c_str())) { path = selected; std::free(selected); }
    #endif

    #if !defined(SR_ANDROID) && !defined(SR_EMSCRIPTEN)
        nfdchar_t *outPath;

        nfdfilteritem_t* pFilterItem = filter.size() > 0 ? new nfdfilteritem_t[filter.size()] : nullptr;

        for (uint32_t i = 0; i < static_cast<uint32_t>(filter.size()); ++i) {
            pFilterItem[i].name = filter[i].first.c_str();
            pFilterItem[i].spec = filter[i].second.c_str();
        }

        SR_UTILS_NS::Path resolvedDefaultPath = defaultPath;
        SR_UTILS_NS::VFS::Instance().ResolvePath(resolvedDefaultPath, FileMode::Read);

        nfdresult_t result = NFD_OpenDialog(&outPath, pFilterItem, static_cast<nfdfiltersize_t>(filter.size()), resolvedDefaultPath.c_str());
        if (result == NFD_OKAY)
        {
            path = String(outPath);
            NFD_FreePath(outPath);
        }
        else if (result == NFD_CANCEL)
        {
            /// ignore
        }
        else
        {
            /// ignore
        }
    #endif

        return path;
    }
}
