#pragma once

#include <RmlUi/Core.h>
#include "engine/application.h"

namespace cqsp::engine {
class CQSPSystemInterface : public Rml::SystemInterface {
 public:
    explicit CQSPSystemInterface(Application& app);
    virtual ~CQSPSystemInterface();

    double GetElapsedTime();
 private:
    Application& m_app;
};
}  // namespace cqsp::engine
