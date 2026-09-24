#pragma once

#include <cstdio>
#include <format>
#include <print>
#include <string_view>
#include <utility>

namespace etude {

    /// @brief Severity of a log message, from informational to fatal.
    enum class LogLevel {
        Info,
        Warning,
        Error,
        Fatal
    };

    /// @brief Returns the lowercase name of a log level, as it appears in the prefix of a log line.
    constexpr std::string_view toString(LogLevel level) {
        switch (level) {
            case LogLevel::Info:
                return "info";
            case LogLevel::Warning:
                return "warning";
            case LogLevel::Error:
                return "error";
            case LogLevel::Fatal:
                return "fatal";
        }
        std::unreachable();
    }

    /// @brief Writes a message with its level to stderr, which is unbuffered, so the line appears even
    /// if the program crashes right after. The format string is checked at compile time, like with std::format.
    template <typename... Args>
    void logMessage(LogLevel level, std::format_string<Args...> format, Args&&... args) {
        std::println(stderr, "[{}] {}", toString(level), std::format(format, std::forward<Args>(args)...));
    }

    /// @brief Logs an informational message about the normal course of the program.
    template <typename... Args>
    void logInfo(std::format_string<Args...> format, Args&&... args) {
        logMessage(LogLevel::Info, format, std::forward<Args>(args)...);
    }

    /// @brief Logs a warning: something unexpected happened, but the program can continue.
    template <typename... Args>
    void logWarning(std::format_string<Args...> format, Args&&... args) {
        logMessage(LogLevel::Warning, format, std::forward<Args>(args)...);
    }

    /// @brief Logs an error: an operation failed and its result is missing or wrong.
    template <typename... Args>
    void logError(std::format_string<Args...> format, Args&&... args) {
        logMessage(LogLevel::Error, format, std::forward<Args>(args)...);
    }

    /// @brief Logs a fatal error: an unrecoverable error occurred and the program has to shut down.
    template <typename... Args>
    void logFatal(std::format_string<Args...> format, Args&&... args) {
        logMessage(LogLevel::Fatal, format, std::forward<Args>(args)...);
    }
}
