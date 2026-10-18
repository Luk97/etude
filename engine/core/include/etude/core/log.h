#pragma once

#include <format>
#include <functional>
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

    /// @brief Receives each log message with its level, such as the console of the editor, which shows the messages in
    /// its window. The message only lives during the call, so a listener that keeps it has to copy it.
    using LogListener = std::function<void(LogLevel level, std::string_view message)>;

    /// @brief Hands every log message from now on to the listener as well, or to none if the function is empty. There
    /// is one listener at a time, so a new one replaces the last. The listener must not log itself, since that message
    /// would come back to it.
    void setLogListener(LogListener listener);

    /// @brief Writes the message with its level to stderr, which is unbuffered, so the line appears even if the
    /// program crashes right after, and then hands it to the listener.
    void writeLog(LogLevel level, std::string_view message);

    /// @brief Formats a message and logs it with its level. The format string is checked at compile time, like with
    /// std::format.
    template <typename... Args>
    void logMessage(LogLevel level, std::format_string<Args...> format, Args&&... args) {
        writeLog(level, std::format(format, std::forward<Args>(args)...));
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
