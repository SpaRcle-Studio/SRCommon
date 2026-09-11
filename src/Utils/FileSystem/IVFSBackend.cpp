//
// Created by Monika on 10.09.2026.
//

#include <Utils/FileSystem/IVFSBackend.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS {
    bool IVFSBackend::IsApplicable(StringView path) const {
        SR_TRACY_ZONE;

        if (SR_PLATFORM_NS::IsAbsolutePath(path)) {
            /// путь указывает прямо внутрь бекенда, поэтому выбран явно, а не разрешением
            /// виртуального пути. Игнорируемые расширения нужны только для того, чтобы бекенд не
            /// перехватывал виртуальные пути, и скрывать явно запрошенный файл они не должны.
            if (!m_realPath.empty() && path.starts_with(m_realPath)) {
                return true;
            }
        }

        if (!m_ignoredExtensions.empty()) {
            if (auto&& dotPos = path.find_last_of('.'); dotPos != StringView::npos) {
                StringView extension = path.substr(dotPos + 1);
                if (m_ignoredExtensions.contains(extension)) {
                    return false;
                }
            }
        }

        if (SR_PLATFORM_NS::IsAbsolutePath(path)) {
            if (m_virtualPath.empty()) {
                return false;
            }
            return path.starts_with(m_virtualPath);
        }
        if (m_virtualPath.empty()) {
            return true;
        }
        return path.starts_with(m_virtualPath);
    }
}