// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        DocumentState.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <cstdint>
#include <expected>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Core {

struct Operation
{
    std::string type;
    std::map<std::string, double> params;
};

class DocumentState
{
  public:
    DocumentState();

    void PushOperation(const Operation& op);
    void Undo();
    void Redo();

    [[nodiscard]] bool CanUndo() const;
    [[nodiscard]] bool CanRedo() const;
    [[nodiscard]] size_t GetCurrentStep() const;
    [[nodiscard]] size_t GetTotalSteps() const;
    [[nodiscard]] const std::vector<Operation>& GetHistory() const;

    void ClearHistory();
    void SetMaxHistory(size_t maxSteps);

    std::expected<std::vector<std::uint8_t>, std::string> Serialize() const;
    std::expected<void, std::string> Deserialize(const std::vector<std::uint8_t>& data);

    std::expected<std::vector<Operation>, std::string> ReplayOperations(const std::vector<Operation>& ops) const;

  private:
    std::vector<Operation> m_history;
    size_t m_currentStep;
    size_t m_maxHistory;
};

} // namespace Core
