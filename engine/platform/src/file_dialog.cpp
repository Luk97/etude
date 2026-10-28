#include <etude/platform/file_dialog.h>

#include "wide_text.h"

#include <cstddef>
#include <string>

#include <windows.h>

#include <commdlg.h>

namespace etude {

    namespace {

        /// @brief How many characters the path of a chosen file may have, far more than MAX_PATH.
        constexpr std::size_t pathLength = 4096;

        /// @brief Returns the filter as the dialog wants it: the name and the pattern, each ending in a zero, and the
        /// list in another one, which c_str adds.
        std::wstring filterText(const FileFilter& filter) {
            return toWide(filter.name) + L'\0' + toWide(filter.pattern) + L'\0';
        }

        /// @brief Returns the window that the dialog belongs to, which it blocks while it is open.
        HWND ownerOf(const Window& owner) {
            return static_cast<HWND>(owner.nativeHandles().window);
        }
    }

    std::optional<std::filesystem::path> openFileDialog(
        const Window& owner,
        const std::filesystem::path& folder,
        const FileFilter& filter
    ) {
        // Without OFN_NOCHANGEDIR, the dialog would move the working directory to the folder of the file.
        const std::wstring filters = filterText(filter);
        const std::wstring start = folder.wstring();
        std::wstring file(pathLength, L'\0');
        OPENFILENAMEW dialog{
            .lStructSize = sizeof(dialog),
            .hwndOwner = ownerOf(owner),
            .lpstrFilter = filters.c_str(),
            .lpstrFile = file.data(),
            .nMaxFile = static_cast<DWORD>(file.size()),
            .lpstrInitialDir = start.c_str(),
            .Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR,
        };
        if (!GetOpenFileNameW(&dialog)) {
            return std::nullopt;
        }
        return std::filesystem::path(file.c_str());
    }

    std::optional<std::filesystem::path> saveFileDialog(
        const Window& owner,
        const std::filesystem::path& folder,
        std::string_view name,
        const FileFilter& filter
    ) {
        // The name fills the field of the dialog, and its extension is the one that the dialog adds.
        const std::wstring filters = filterText(filter);
        const std::wstring start = folder.wstring();
        const std::wstring suggested = toWide(name);
        const std::wstring extension = std::filesystem::path(suggested).extension().wstring();
        std::wstring file(pathLength, L'\0');
        file.replace(0, suggested.size(), suggested);
        OPENFILENAMEW dialog{
            .lStructSize = sizeof(dialog),
            .hwndOwner = ownerOf(owner),
            .lpstrFilter = filters.c_str(),
            .lpstrFile = file.data(),
            .nMaxFile = static_cast<DWORD>(file.size()),
            .lpstrInitialDir = start.c_str(),
            .Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR,
            .lpstrDefExt = extension.empty() ? nullptr : extension.c_str() + 1,
        };
        if (!GetSaveFileNameW(&dialog)) {
            return std::nullopt;
        }
        return std::filesystem::path(file.c_str());
    }
}
