#include "rmlsysteminterface.h"

#include "common/util/logging.h"

cqsp::engine::CQSPSystemInterface::CQSPSystemInterface(Application& app) : m_app(app) {
    logger = cqsp::common::util::make_logger("RmlUi", true);
}

cqsp::engine::CQSPSystemInterface::~CQSPSystemInterface() {}

double cqsp::engine::CQSPSystemInterface::GetElapsedTime() {
    return m_app.GetTime();
}

bool cqsp::engine::CQSPSystemInterface::LogMessage(Rml::Log::Type type,
                                                   const Rml::String& message) {
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
            level = spdlog::level::level_enum::warn;
            break;
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
