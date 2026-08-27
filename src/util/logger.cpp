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
#include <thread>

#if defined(__APPLE__) || defined(__MACH__)
#include <sys/sysctl.h>
#include <sys/utsname.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

namespace
{
    // The logger shares a single mutex across all log calls so concurrent model
    // solves may emit messages without interleaving corruptly.
    std::mutex& loggerMutex()
    {
        static std::mutex mutex;
        return mutex;
    }

    [[nodiscard]] std::string formatTimestamp()
    {
        // Use system_clock for wall-clock time. time_t is the seconds-based value
        // required by localtime_*; this conversion is what allows formatted timestamps.
        const auto now = std::chrono::system_clock::now();
        // 获取当前时间点，返回一个 std::chrono::time_point 对象，表示从系统时钟的纪元（通常是 1970 年 1 月 1 日）到现在的时间间隔
        const auto time = std::chrono::system_clock::to_time_t(now);
        // time_t 是一个整数类型，表示自 1970 年 1 月 1 日以来的秒数
        std::tm local_time{}; // std::tm 是一个结构体，表示本地时间的各个组成部分，如年、月、日、时、分、秒等
#if defined(_MSC_VER) // if MSVC, use localtime_s for thread safety
        localtime_s(&local_time, &time);
#else
        localtime_r(&time, &local_time); // if not MSVC, use localtime_r for thread safety
#endif

        std::ostringstream oss;
        oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
        // std::put_time 是一个操纵符，用于将时间格式化为指定的字符串形式
        return oss.str();
    }

    // std::ofstream is a subclass of std::ostream, which represents an output stream that writes to a file.
    // that is why we can use std::ofstream to write log messages to a file, and std::ostream to write log messages to the console or other output streams.
    void writeLogLine(std::ostream& output,
                      const LogLevel level,
                      const bool include_timestamp,
                      const bool include_source_location,
                      const std::string_view message,
                      const std::source_location& location)
    {
        if (include_timestamp)
        {
            output << "[" << formatTimestamp() << "] ";
        }
        if (level != LogLevel::Off && level != LogLevel::Info)
            output << "[" << toString(level) << "] ";
        output << message;
        if (include_source_location)
        {
            output << " (" << location.file_name() << ":" << location.line() << ")";
        }
        output << '\n';
        output.flush();
        // flush the output stream to ensure that the log message is written immediately
    }

    [[nodiscard]] unsigned int getPhysicalCores()
    {
#if defined(__APPLE__) || defined(__MACH__)
        int count = 0;
        size_t size = sizeof(count);
        if (sysctlbyname("hw.physicalcpu", &count, &size, nullptr, 0) == 0)
        {
            return static_cast<unsigned int>(count);
        }
#elif defined(_WIN32)
        SYSTEM_INFO info{};
        GetSystemInfo(&info);
        const DWORD logical = info.dwNumberOfProcessors;
        return logical > 0 ? static_cast<unsigned int>(logical) : 1;
#endif
        return 1;
    }

    [[nodiscard]] unsigned int getLogicalCores()
    {
        const unsigned int count = std::thread::hardware_concurrency();
        return count > 0 ? count : 1;
    }

    [[nodiscard]] std::string getCpuModel()
    {
#if defined(__APPLE__) || defined(__MACH__)
        char buffer[256] = {};
        size_t size = sizeof(buffer);
        if (sysctlbyname("machdep.cpu.brand_string", buffer, &size, nullptr, 0) == 0)
        {
            return std::string(buffer);
        }
#elif defined(_WIN32)
        char buffer[256] = {};
        const DWORD size = GetEnvironmentVariableA("PROCESSOR_IDENTIFIER", buffer, sizeof(buffer));
        if (size > 0 && size < sizeof(buffer))
        {
            return std::string(buffer);
        }
        SYSTEM_INFO info{};
        GetNativeSystemInfo(&info);
        switch (info.wProcessorArchitecture)
        {
        case PROCESSOR_ARCHITECTURE_AMD64:
            return "AMD64 / x86_64";
        case PROCESSOR_ARCHITECTURE_ARM:
            return "ARM";
        case PROCESSOR_ARCHITECTURE_ARM64:
            return "ARM64";
        case PROCESSOR_ARCHITECTURE_INTEL:
            return "x86";
        default:
            return "Unknown CPU";
        }
#endif
        return "Unknown CPU";
    }

    [[nodiscard]] std::string getPlatformName()
    {
#if defined(__APPLE__) || defined(__MACH__)
        utsname info{};
        if (uname(&info) == 0)
        {
            return std::string(info.sysname) + " " + info.release + " " + info.machine;
        }
#elif defined(_WIN32)
        OSVERSIONINFOA info{};
        info.dwOSVersionInfoSize = sizeof(info);
        if (GetVersionExA(&info))
        {
            std::ostringstream oss;
            oss << "Windows " << info.dwMajorVersion << "." << info.dwMinorVersion;
            if (info.szCSDVersion[0] != '\0')
            {
                oss << " " << info.szCSDVersion;
            }
            return oss.str();
        }
        return "Windows";
#else
        utsname info{};
        if (uname(&info) == 0)
        {
            return std::string(info.sysname) + " " + info.release + " " + info.machine;
        }
#endif
        return "Unknown platform";
    }
} // namespace

const char* toString(const LogLevel level) noexcept
{
    switch (level)
    {
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

std::string Logger::getInfo() noexcept
{
    // std::ostringstream 和 std::cout 的区别是：std::cout：直接往终端/输出流写
    // std::ostringstream 先往内存里的字符串流里写，最后 oss.str() 取出
    std::ostringstream oss; // 创建一个字符串流对象 oss，用于将多个信息拼接成一个字符串
    const unsigned int physical = getPhysicalCores();
    const unsigned int logical = getLogicalCores();

    // CHEN_SOLVER_VERSION 是 cmakelist 中定义的宏，表示当前求解器的版本号
    oss << "Chen Solver version " << CHEN_SOLVER_VERSION << "\n";
    oss << "Build: " << __DATE__ << " " << __TIME__ << "\n";
    oss << "Platform: " << getPlatformName() << "\n";
    oss << "CPU model: " << getCpuModel() << "\n";
    oss << "Thread count: " << physical << " physical cores, " << logical
        << " logical processors, using up to " << logical << " threads\n";
    return oss.str();
}

Logger& Logger::instance() noexcept
{
    static Logger logger;
    if (logger.output_ == nullptr)
    {
        // 表示还没有设置输出流，即第一次调用 instance 时，默认使用 std::cout
        logger.output_ = &std::cout; // clog 是标准库提供的一个日志输出流对象，通常用于输出日志信息，默认输出到标准错误流（stderr），红色字体
        // 其他输出流还有 std::cout（标准输出流，通常用于输出普通信息）和 std::cerr（标准错误流，通常用于输出错误信息）
        // 文件流 std::ofstream，字符串流 std::ostringstream 等
    }
    return logger;
}

// 如果多个线程同时调用 setLevel()，setOutputStream() 等函数，就可能出现数据竞争和不一致的状态。为了保证线程安全，
// 需要在这些函数中使用互斥锁（mutex）来保护共享资源的访问。std::lock_guard 是一个 RAII 风格的互斥锁管理类，
// 它在构造时锁定互斥锁，在析构时自动释放锁，
// 确保在函数退出时自动释放锁，避免死锁和资源泄漏。
void Logger::setLevel(const LogLevel level) noexcept
{
    std::lock_guard lock(loggerMutex()); // 使用 std::lock_guard 来管理互斥锁的生命周期，确保在函数退出时自动释放锁，避免死锁和资源泄漏
    level_ = level;
}

LogLevel Logger::level() const noexcept
{
    std::lock_guard lock(loggerMutex());
    return level_;
}

void Logger::setOutputStream(std::ostream& stream) noexcept
{
    std::lock_guard lock(loggerMutex());
    output_ = &stream;
}

void Logger::writeLogFile(const std::string& path)
{
    std::lock_guard lock(loggerMutex());
    // 关闭之前的文件流，确保不会同时写入多个文件
    file_output_.close();
    // 以追加模式打开文件，如果文件不存在则创建新文件
    // out 表示写入模式，app 表示追加模式
    file_output_.open(path, std::ios::out | std::ios::app);
}

void Logger::enableTimestamps(const bool enable) noexcept
{
    std::lock_guard lock(loggerMutex());
    timestamps_enabled_ = enable;
}

void Logger::enableSourceLocation(const bool enable) noexcept
{
    std::lock_guard lock(loggerMutex());
    source_location_enabled_ = enable;
}

bool Logger::timestampsEnabled() const noexcept
{
    std::lock_guard lock(loggerMutex());
    return timestamps_enabled_;
}

bool Logger::sourceLocationEnabled() const noexcept
{
    std::lock_guard lock(loggerMutex());
    return source_location_enabled_;
}

void Logger::reset() noexcept
{
    std::lock_guard lock(loggerMutex());
    level_ = LogLevel::Info;
    output_ = &std::cout;
    file_output_.close();
    timestamps_enabled_ = true;
    source_location_enabled_ = false;
}

// 判断是否应该记录日志，主要是根据当前日志级别和传入的日志级别进行比较
bool Logger::shouldLog(const LogLevel level) const noexcept
{
    std::lock_guard lock(loggerMutex());
    return level != LogLevel::Off && level_ != LogLevel::Off &&
        static_cast<int>(level) >= static_cast<int>(level_);
}

void Logger::log(const LogLevel level,
                 const std::string_view message,
                 const std::source_location& location)
{
    std::lock_guard lock(loggerMutex());
    if (level == LogLevel::Off || level_ == LogLevel::Off ||
        static_cast<int>(level) < static_cast<int>(level_))
    {
        return;
    }
    if (output_ == nullptr)
    {
        output_ = &std::cout;
    }

    writeLogLine(*output_, level, timestamps_enabled_, source_location_enabled_, message,
                 location);
    if (file_output_.is_open())
    {
        // 如果文件流已经打开，则将日志写入文件
        writeLogLine(file_output_, level, timestamps_enabled_, source_location_enabled_,
                     message, location);
    }
}

void Logger::logHeader(const std::string_view header,
                       const std::source_location& location)
{
    log(LogLevel::Info, std::string(header), location);
}

void Logger::trace(const std::string_view message, const std::source_location& location)
{
    log(LogLevel::Trace, message, location);
}

void Logger::debug(const std::string_view message, const std::source_location& location)
{
    log(LogLevel::Debug, message, location);
}

void Logger::info(const std::string_view message, const std::source_location& location)
{
    log(LogLevel::Info, message, location);
}

void Logger::warn(const std::string_view message, const std::source_location& location)
{
    log(LogLevel::Warn, message, location);
}

void Logger::error(const std::string_view message, const std::source_location& location)
{
    log(LogLevel::Error, message, location);
}
