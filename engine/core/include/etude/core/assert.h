#pragma once

#include <cstdio>
#include <cstdlib>
#include <print>
#include <source_location>

namespace etude {

    /// @brief Reports a failed assertion with file, line and function, then aborts the program.
    /// Called by ETUDE_ASSERT. The default argument captures the location of the assertion, not of this function.
    [[noreturn]] inline void assertFailed(
        const char* expression,
        std::source_location location = std::source_location::current()
    ) {
        std::println(
            stderr, "[assert] {}:{}: {} failed in {}", location.file_name(), location.line(), expression,
            location.function_name()
        );
        std::abort();
    }

#ifdef NDEBUG
    #define ETUDE_ASSERT(expression) static_cast<void>(0)
#else
    #define ETUDE_ASSERT(expression) ((expression) ? static_cast<void>(0) : ::etude::assertFailed(#expression))
#endif
}
