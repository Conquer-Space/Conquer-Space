#include "rmlsysteminterface.h"

cqsp::engine::CQSPSystemInterface::CQSPSystemInterface(Application& app) : m_app(app) {
}

cqsp::engine::CQSPSystemInterface::~CQSPSystemInterface() {}

double cqsp::engine::CQSPSystemInterface::GetElapsedTime() {
    return m_app.GetTime();
}
