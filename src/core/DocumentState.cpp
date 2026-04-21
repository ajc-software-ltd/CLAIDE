// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        DocumentState.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/DocumentState.hpp"

#include <algorithm>
#include <cstring>
#include <sstream>

#include <spdlog/spdlog.h>

namespace Core {

DocumentState::DocumentState() : m_currentStep(0), m_maxHistory(20) {
}

void DocumentState::PushOperation(const Operation& op) {
    if (m_currentStep < m_history.size()) {
        m_history.erase(m_history.begin() + m_currentStep, m_history.end());
    }

    m_history.push_back(op);
    m_currentStep = m_history.size();

    if (m_history.size() > m_maxHistory) {
        m_history.erase(m_history.begin());
        m_currentStep = m_history.size();
    }

    spdlog::debug("DocumentState::PushOperation: {} (step {}/{})", op.type, m_currentStep, m_history.size());
}

void DocumentState::Undo() {
    if (m_currentStep > 0) {
        m_currentStep--;
        spdlog::debug("DocumentState::Undo: step {}/{}", m_currentStep, m_history.size());
    }
}

void DocumentState::Redo() {
    if (m_currentStep < m_history.size()) {
        m_currentStep++;
        spdlog::debug("DocumentState::Redo: step {}/{}", m_currentStep, m_history.size());
    }
}

bool DocumentState::CanUndo() const {
    return m_currentStep > 0;
}

bool DocumentState::CanRedo() const {
    return m_currentStep < m_history.size();
}

size_t DocumentState::GetCurrentStep() const {
    return m_currentStep;
}

size_t DocumentState::GetTotalSteps() const {
    return m_history.size();
}

const std::vector<Operation>& DocumentState::GetHistory() const {
    return m_history;
}

void DocumentState::ClearHistory() {
    m_history.clear();
    m_currentStep = 0;
    spdlog::debug("DocumentState::ClearHistory");
}

void DocumentState::SetMaxHistory(size_t maxSteps) {
    m_maxHistory = maxSteps;
    if (m_history.size() > m_maxHistory) {
        m_history.erase(m_history.begin(), m_history.begin() + (m_history.size() - m_maxHistory));
        m_currentStep = std::min(m_currentStep, m_history.size());
    }
}

std::expected<std::vector<std::uint8_t>, std::string> DocumentState::Serialize() const {
    std::vector<std::uint8_t> data;
    std::ostringstream oss;

    oss << m_currentStep << "\n" << m_maxHistory << "\n" << m_history.size() << "\n";

    for (const auto& op : m_history) {
        oss << op.type << "\n" << op.params.size() << "\n";
        for (const auto& [key, value] : op.params) {
            oss << key << " " << value << "\n";
        }
    }

    std::string str = oss.str();
    data.assign(str.begin(), str.end());

    spdlog::debug("DocumentState::Serialize: {} bytes, {} operations", data.size(), m_history.size());
    return data;
}

std::expected<void, std::string> DocumentState::Deserialize(const std::vector<std::uint8_t>& data) {
    std::string str(data.begin(), data.end());
    std::istringstream iss(str);

    size_t historySize = 0;
    if (!(iss >> m_currentStep >> m_maxHistory >> historySize)) {
        return std::unexpected("Failed to parse DocumentState header");
    }

    m_history.clear();
    for (size_t i = 0; i < historySize; ++i) {
        Operation op;
        size_t paramCount = 0;
        if (!(iss >> op.type >> paramCount)) {
            return std::unexpected("Failed to parse operation");
        }

        for (size_t j = 0; j < paramCount; ++j) {
            std::string key;
            double value = 0.0;
            if (!(iss >> key >> value)) {
                return std::unexpected("Failed to parse operation parameter");
            }
            op.params[key] = value;
        }
        m_history.push_back(std::move(op));
    }

    spdlog::debug("DocumentState::Deserialize: {} operations loaded", m_history.size());
    return {};
}

std::expected<std::vector<Operation>, std::string>
DocumentState::ReplayOperations(const std::vector<Operation>& ops) const {
    spdlog::debug("DocumentState::ReplayOperations: {} operations", ops.size());
    return ops;
}

} // namespace Core
