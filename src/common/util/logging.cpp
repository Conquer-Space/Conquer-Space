/* Conquer Space
* Copyright (C) 2021 Conquer Space
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "common/util/logging.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/dup_filter_sink.h>

#include <filesystem>
#include <vector>
#include <string>

#include "common/util/paths.h"

// Change this if you need
static const char* DEFAULT_PATTERN = "[%T.%e] [%^%l%$] [%n] [%s:%#] %v";

std::shared_ptr<spdlog::logger> cqsp::common::util::make_logger(std::string name, bool error) {
    std::shared_ptr<spdlog::logger> logger;

    // Get log folder
    std::string save_path = GetCqspSavePath();
    std::filesystem::path log_folder = std::filesystem::path(save_path) / "logs";

    auto dup_filter = std::make_shared<spdlog::sinks::dup_filter_sink_st>(std::chrono::seconds(10));

    // Initialize logger
    std::vector<spdlog::sink_ptr> sinks;
    if (error) {
        std::string log_file_name = (log_folder / (name + ".error.txt")).string();
        auto error_log = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_name, true);
        error_log->set_level(spdlog::level::err);
        sinks.push_back(error_log);
    }

#ifdef NDEBUG
    // flush to file every 3 seconds so that we can see it
    spdlog::flush_every(std::chrono::seconds(3));
    std::string log_name = (log_folder / (name + ".txt")).string();
    auto basic_logger = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_name, true);
    sinks.push_back(basic_logger);
#else
    auto console_logger = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sinks.push_back(console_logger);
#endif
    dup_filter->set_sinks(sinks);
    logger = std::make_shared<spdlog::logger>(name, dup_filter);

    // Default pattern
    logger->set_pattern(DEFAULT_PATTERN);
    return logger;
}
