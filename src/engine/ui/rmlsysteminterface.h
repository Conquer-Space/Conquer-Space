#pragma once

#include <RmlUi/Core.h>
#include <spdlog/spdlog.h>

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
