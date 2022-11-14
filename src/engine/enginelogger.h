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
#pragma once

#include <spdlog/spdlog.h>

#include <memory>

namespace cqsp::engine {
extern std::shared_ptr<spdlog::logger> engine_logger;
}  // namespace cqsp::engine

#define ENGINE_LOG_TRACE(...) SPDLOG_LOGGER_TRACE(cqsp::engine::engine_logger, __VA_ARGS__)
#define ENGINE_LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(cqsp::engine::engine_logger, __VA_ARGS__)
#define ENGINE_LOG_INFO(...) SPDLOG_LOGGER_INFO(cqsp::engine::engine_logger, __VA_ARGS__)
#define ENGINE_LOG_WARN(...) SPDLOG_LOGGER_TRACE(cqsp::engine::engine_logger, __VA_ARGS__)
#define ENGINE_LOG_ERROR(...) SPDLOG_LOGGER_TRACE(cqsp::engine::engine_logger, __VA_ARGS__)
#define ENGINE_LOG_CRITICAL(...) SPDLOG_LOGGER_TRACE(cqsp::engine::engine_logger, __VA_ARGS__)
