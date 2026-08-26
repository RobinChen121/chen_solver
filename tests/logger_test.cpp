/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/20, 21:38
 * Description: 
 *
 */

#include <cassert>
#include <sstream>
#include <string>

#include "chen_solver/util/logger.h"

int main() {
    auto &logger = Logger::instance(); {
        std::ostringstream oss;
        logger.reset();
        logger.setOutputStream(oss);
        logger.enableTimestamps(false);
        logger.enableSourceLocation(false);
        logger.setLevel(LogLevel::Info);

        logger.debug("hidden debug line");
        logger.warn("tightened variable bound");

        assert(oss.str() == "[WARN] tightened variable bound\n");
    } {
        std::ostringstream oss;
        logger.reset();
        logger.setOutputStream(oss);
        logger.enableTimestamps(false);
        logger.enableSourceLocation(true);
        logger.setLevel(LogLevel::Trace);

        logger.info("presolve started");

        const std::string output = oss.str();
        assert(output.find("presolve started") != std::string::npos);
        assert(output.find("logger_test.cpp:") != std::string::npos);
        assert(output.find("[INFO]") == std::string::npos);
    }

    logger.reset();
    return 0;
}
