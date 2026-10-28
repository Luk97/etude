#pragma once

#include <etude/platform/file_filter.h>
#include <etude/platform/window.h>

#include <filesystem>
#include <optional>
#include <string_view>

namespace etude {

    /// @brief Shows the dialog of Windows for opening a file above the window, starting in the folder and showing the
    /// files of the filter. Returns the chosen file, or nothing if the user cancels.
    std::optional<std::filesystem::path> openFileDialog(
        const Window& owner,
        const std::filesystem::path& folder,
        const FileFilter& filter
    );

    /// @brief Shows the dialog of Windows for saving a file above the window, starting in the folder with the name
    /// filled in, whose extension the dialog adds to a name that the user types without one. Before it replaces a file,
    /// the dialog asks. Returns the chosen file, or nothing if the user cancels.
    std::optional<std::filesystem::path> saveFileDialog(
        const Window& owner,
        const std::filesystem::path& folder,
        std::string_view name,
        const FileFilter& filter
    );
}
