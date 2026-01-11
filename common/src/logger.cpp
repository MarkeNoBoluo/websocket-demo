#include "ws_common/interface.hpp"
#include <iostream>

namespace ws {

class Logger {
public:
    static void info(const std::string& msg) {
        std::cout << "[INFO] " << msg << std::endl;
    }
    
    static void error(const std::string& msg) {
        std::cerr << "[ERROR] " << msg << std::endl;
    }
};

} // namespace ws