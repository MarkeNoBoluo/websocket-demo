#include "ws_common/logger.hpp"
#include <chrono>
#include <iomanip>
#include <ctime>

namespace KK_WS {

std::mutex Logger::log_mutex_;
Logger::Level Logger::current_level_ = Level::Ws_INFO;

const char* Logger::level_to_string(Level level) {
    switch (level) {
        case Level::Ws_DEBUG:   return "DEBUG";
        case Level::Ws_INFO:    return "INFO";
        case Level::Ws_WARNING: return "WARN";
        case Level::Ws_ERROR:   return "ERROR";
        default:             return "UNKNOWN";
    }
}

void Logger::log(Level level, const std::string& msg) {
    if (level < current_level_) {
        return;
    }

    std::lock_guard<std::mutex> lock(log_mutex_);
    
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t);
#else
    localtime_r(&time_t, &tm_buf);
#endif
    
    // 输出格式: [时间] [级别] 消息
    std::cout << "["
              << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
              << "." << std::setfill('0') << std::setw(3) << ms.count()
              << "] ["
              << level_to_string(level)
              << "] "
              << msg
              << std::endl;
}

void Logger::debug(const std::string& msg) {
    log(Level::Ws_DEBUG, msg);
}

void Logger::info(const std::string& msg) {
    log(Level::Ws_INFO, msg);
}

void Logger::warning(const std::string& msg) {
    log(Level::Ws_WARNING, msg);
}

void Logger::error(const std::string& msg) {
    log(Level::Ws_ERROR, msg);
}

void Logger::set_level(Level level) {
    current_level_ = level;
}

} // namespace ws
