/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/20, 21:38
 * Description: 
 *
 */

#include "chen_solver/util/logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace chen_solver {
    namespace {
        std::mutex &loggerMutex() {
            static std::mutex mutex;
            return mutex;
        }

        [[nodiscard]] std::string formatTimestamp() {
            const auto now = std::chrono::system_clock::now();
            const auto time = std::chrono::system_clock::to_time_t(now);
            std::tm local_time{};
#if defined(_MSC_VER)
            localtime_s(&local_time, &time);
#else
            localtime_r(&time, &local_time);
#endif

            std::ostringstream oss;
            oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }

        void writeLogLine(std::ostream &output,
                          const LogLevel level,
                          const bool include_timestamp,
                          const bool include_source_location,
                          const std::string_view message,
                          const std::source_location &location) {
            if (include_timestamp) {
                output << "[" << formatTimestamp() << "] ";
            }
            output << "[" << toString(level) << "] " << message;
            if (include_source_location) {
                output << " (" << location.file_name() << ":" << location.line() << ")";
            }
            output << '\n';
            output.flush();
        }
    } // namespace

    const char *toString(const LogLevel level) noexcept {
        switch (level) {
            case LogLevel::Trace:
                return "TRACE";
            case LogLevel::Debug:
                return "DEBUG";
            case LogLevel::Info:
                return "INFO";
            case LogLevel::Warn:
                return "WARN";
            case LogLevel::Error:
                return "ERROR";
            case LogLevel::Off:
                return "OFF";
        }
        return "UNKNOWN";
    }

    Logger &Logger::instance() noexcept {
        static Logger logger;
        if (logger.output_ == nullptr) {
            logger.output_ = &std::clog;
        }
        return logger;
    }

    void Logger::setLevel(const LogLevel level) noexcept {
        std::lock_guard lock(loggerMutex());
        level_ = level;
    }

    LogLevel Logger::level() const noexcept {
        std::lock_guard lock(loggerMutex());
        return level_;
    }

    void Logger::setOutputStream(std::ostream &stream) noexcept {
        std::lock_guard lock(loggerMutex());
        output_ = &stream;
    }

    void Logger::enableTimestamps(const bool enable) noexcept {
        std::lock_guard lock(loggerMutex());
        timestamps_enabled_ = enable;
    }

    void Logger::enableSourceLocation(const bool enable) noexcept {
        std::lock_guard lock(loggerMutex());
        source_location_enabled_ = enable;
    }

    bool Logger::timestampsEnabled() const noexcept {
        std::lock_guard lock(loggerMutex());
        return timestamps_enabled_;
    }

    bool Logger::sourceLocationEnabled() const noexcept {
        std::lock_guard lock(loggerMutex());
        return source_location_enabled_;
    }

    void Logger::reset() noexcept {
        std::lock_guard lock(loggerMutex());
        level_ = LogLevel::Info;
        output_ = &std::clog;
        timestamps_enabled_ = true;
        source_location_enabled_ = false;
    }

    bool Logger::shouldLog(const LogLevel level) const noexcept {
        std::lock_guard lock(loggerMutex());
        return level != LogLevel::Off && level_ != LogLevel::Off &&
               static_cast<int>(level) >= static_cast<int>(level_);
    }

    void Logger::log(const LogLevel level,
                     const std::string_view message,
                     const std::source_location &location) {
        std::lock_guard lock(loggerMutex());
        if (level == LogLevel::Off || level_ == LogLevel::Off ||
            static_cast<int>(level) < static_cast<int>(level_)) {
            return;
        }
        if (output_ == nullptr) {
            output_ = &std::clog;
        }
        writeLogLine(*output_, level, timestamps_enabled_, source_location_enabled_, message,
                     location);
    }

    void Logger::trace(const std::string_view message, const std::source_location &location) {
        log(LogLevel::Trace, message, location);
    }

    void Logger::debug(const std::string_view message, const std::source_location &location) {
        log(LogLevel::Debug, message, location);
    }

    void Logger::info(const std::string_view message, const std::source_location &location) {
        log(LogLevel::Info, message, location);
    }

    void Logger::warn(const std::string_view message, const std::source_location &location) {
        log(LogLevel::Warn, message, location);
    }

    void Logger::error(const std::string_view message, const std::source_location &location) {
        log(LogLevel::Error, message, location);
    }
} // namespace chen_solver
