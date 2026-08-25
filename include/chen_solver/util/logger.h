/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/20, 21:38
 * Description: 
 *
 */

#ifndef CHEN_SOLVER_UTIL_LOGGER_H
#define CHEN_SOLVER_UTIL_LOGGER_H

#include <iosfwd>
#include <source_location>
#include <string_view>

#include "chen_solver/config.h"

namespace chen_solver {
    enum class LogLevel : uint8_t {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warn = 3,
        Error = 4,
        Off = 5,
    };

    [[nodiscard]] const char *toString(const LogLevel level) noexcept;

    class Logger {
    public:
        static Logger &instance() noexcept;

        void setLevel(LogLevel level) noexcept;
        [[nodiscard]] LogLevel level() const noexcept;

        void setOutputStream(std::ostream &stream) noexcept;
        void enableTimestamps(bool enable) noexcept;
        void enableSourceLocation(bool enable) noexcept;
        [[nodiscard]] bool timestampsEnabled() const noexcept;
        [[nodiscard]] bool sourceLocationEnabled() const noexcept;

        void reset() noexcept;
        [[nodiscard]] bool shouldLog(LogLevel level) const noexcept;

        void log(LogLevel level,
                 std::string_view message,
                 const std::source_location &location = std::source_location::current());
        void trace(std::string_view message,
                   const std::source_location &location = std::source_location::current());
        void debug(std::string_view message,
                   const std::source_location &location = std::source_location::current());
        void info(std::string_view message,
                  const std::source_location &location = std::source_location::current());
        void warn(std::string_view message,
                  const std::source_location &location = std::source_location::current());
        void error(std::string_view message,
                   const std::source_location &location = std::source_location::current());

    private:
        Logger() = default;

        LogLevel level_{LogLevel::Info};
        std::ostream *output_{nullptr};
        bool timestamps_enabled_{true};
        bool source_location_enabled_{false};
    };
} // namespace chen_solver

#endif // CHEN_SOLVER_UTIL_LOGGER_H
