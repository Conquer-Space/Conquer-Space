/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include "engine/ui/rmlsysteminterface.h"

#include "common/util/logging.h"

cqsp::engine::CQSPSystemInterface::CQSPSystemInterface(Application& app) : m_app(app) {
    logger = cqsp::common::util::make_registered_logger("RmlUi", true);
}

cqsp::engine::CQSPSystemInterface::~CQSPSystemInterface() = default;

double cqsp::engine::CQSPSystemInterface::GetElapsedTime() { return m_app.GetTime(); }

bool cqsp::engine::CQSPSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message) {
    spdlog::level::level_enum level;
    switch (type) {
        case Rml::Log::Type::LT_MAX:
            level = spdlog::level::level_enum::trace;
            break;
        case Rml::Log::Type::LT_INFO:
            level = spdlog::level::level_enum::info;
            break;
        case Rml::Log::Type::LT_DEBUG:
            level = spdlog::level::level_enum::debug;
            break;
        case Rml::Log::Type::LT_WARNING:
        case Rml::Log::Type::LT_ASSERT:
            level = spdlog::level::level_enum::warn;
            break;
        case Rml::Log::Type::LT_ERROR:
            level = spdlog::level::level_enum::err;
            break;
        case Rml::Log::Type::LT_ALWAYS:
            level = spdlog::level::level_enum::critical;
            break;
    }
    SPDLOG_LOGGER_CALL(logger, level, "RML: {}", message);
    return true;
}
