#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <mutex>

namespace KK_WS {

/**
 * @brief 简单的线程安全日志类
 */
class Logger {
public:
    enum class Level {
        Ws_DEBUG,
        Ws_INFO,
        Ws_WARNING,
        Ws_ERROR
    };

    static void debug(const std::string& msg);
    static void info(const std::string& msg);
    static void warning(const std::string& msg);
    static void error(const std::string& msg);
    
    static void set_level(Level level);

private:
    static void log(Level level, const std::string& msg);
    static std::mutex log_mutex_;
    static Level current_level_;
    static const char* level_to_string(Level level);
};

} // namespace ws
