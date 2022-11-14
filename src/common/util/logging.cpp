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

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/dup_filter_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

#include <tracy/Tracy.hpp>

#include "common/util/paths.h"

namespace {
// Change this if you need
static const char* DEFAULT_PATTERN = "[%T.%e] [%^%l%$] [%n] [%s:%#] %v";

template <typename Mutex>
class TracySink : public spdlog::sinks::base_sink<Mutex> {
 protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        std::string tracy_msg = fmt::to_string(formatted);
        TracyMessage(tracy_msg.c_str(), tracy_msg.size());
    }

    void flush_() override {}
};

using TracySink_mt = TracySink<std::mutex>;
using TracySink_st = TracySink<spdlog::details::null_mutex>;
}  // namespace

namespace cqsp::common::util {
std::shared_ptr<spdlog::logger> make_logger(const std::string& name, bool error) {
    std::shared_ptr<spdlog::logger> logger;

    // Get log folder
    std::string save_path = GetCqspSavePath();
    std::filesystem::path log_folder = std::filesystem::path(save_path) / "logs";

    auto dup_filter = std::make_shared<spdlog::sinks::dup_filter_sink_mt>(std::chrono::seconds(10));

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

#if TRACY_ENABLE
    sinks.push_back(std::make_shared<TracySink_mt>());
#endif  // TRACY_ENABLE

    if (spdlog::get_level() == spdlog::level::off) {
        for (auto& sink : sinks) {
            sink->set_level(spdlog::level::off);
        }
    }
    dup_filter->set_sinks(sinks);
    logger = std::make_shared<spdlog::logger>(name, dup_filter);

    // Default pattern
    logger->set_pattern(DEFAULT_PATTERN);
    return logger;
}

std::shared_ptr<spdlog::logger> make_registered_logger(const std::string& name, bool error) {
    auto logger = make_logger(name, error);
    spdlog::register_logger(logger);
    return logger;
}
}  // namespace cqsp::common::util
