#pragma once

#include <string>

namespace etude {

    /// @brief The kind of file that a file dialog offers: a name that the dialog shows, such as Szenen, and a pattern
    /// that picks the files, such as *.json, or several patterns separated by semicolons.
    struct FileFilter {
        std::string name;
        std::string pattern;
    };
}
