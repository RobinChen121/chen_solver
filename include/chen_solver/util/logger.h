/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/20, 21:38
 * Description: 
 *
 */

#ifndef CHEN_SOLVER_UTIL_LOGGER_H
#define CHEN_SOLVER_UTIL_LOGGER_H

#include <fstream>
#include <iosfwd>
#include <source_location>
#include <string_view>

#include "chen_solver/config.h"

enum class LogLevel : uint8_t
{
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Off = 5,
};

// 输出为 char * 比 string 更轻量级，避免了 string 的内存分配和析构开销，因为
// 当前字符串固定为常量，不需要拼接修改，直接用 char * 更高效
[[nodiscard]] const char* toString(const LogLevel level) noexcept;

class Logger
{
    Logger() = default;

    LogLevel level_{LogLevel::Info};
    // 这里用指针，因为它可以指向不同的输出流对象，std::ostream 是一个抽象类，不能直接实例化对象
    std::ostream* output_{nullptr}; // 输出到终端的流对象
    std::ofstream file_output_; // 输出到文件的流对象，它是 ostream 的子类，表示文件输出流
    bool timestamps_enabled_{false};
    bool source_location_enabled_{false};

public:
    // 用 static 这个对象在第一次调用时创建
    // 之后整个程序复用同一个对象
    // 生命周期和程序相同。
    // 返回引用，这样可以避免拷贝构造函数和析构函数的调用，保证只有一个实例存在
    static Logger& instance() noexcept;
    [[nodiscard]] static std::string getInfo() noexcept;

    void setLevel(LogLevel level) noexcept;
    [[nodiscard]] LogLevel level() const noexcept;

    void setOutputStream(std::ostream& stream) noexcept;
    void writeLogFile(const std::string& path);
    void enableTimestamps(bool enable) noexcept;
    void enableSourceLocation(bool enable) noexcept;
    [[nodiscard]] bool timestampsEnabled() const noexcept;
    [[nodiscard]] bool sourceLocationEnabled() const noexcept;

    void reset() noexcept;
    [[nodiscard]] bool shouldLog(LogLevel level) const noexcept;

    void log(LogLevel level,
             std::string_view message,
             const std::source_location& location = std::source_location::current());
    void logHeader(std::string_view header,
                   const std::source_location& location = std::source_location::current());
    void trace(std::string_view message,
               const std::source_location& location = std::source_location::current());
    void debug(std::string_view message,
               const std::source_location& location = std::source_location::current());
    void info(std::string_view message,
              const std::source_location& location = std::source_location::current());
    void warn(std::string_view message,
              const std::source_location& location = std::source_location::current());
    void error(std::string_view message,
               const std::source_location& location = std::source_location::current());
};
#endif // CHEN_SOLVER_UTIL_LOGGER_H
