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

#include <RmlUi/Core.h>
#include <spdlog/spdlog.h>

#include <memory>

#include "engine/application.h"

namespace cqsp::engine {
class CQSPSystemInterface : public Rml::SystemInterface {
 public:
    explicit CQSPSystemInterface(Application& app);
    virtual ~CQSPSystemInterface();

    double GetElapsedTime();
    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
 private:
    Application& m_app;
    std::shared_ptr<spdlog::logger> logger;
};
}  // namespace cqsp::engine
