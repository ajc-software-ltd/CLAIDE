#pragma once

#include <string>
#include <vector>

namespace Render {

class VulkanDebug
{
  public:
    void EnableValidation(bool enabled);
    bool IsValidationEnabled() const;

    void AddMessage(std::string message);
    const std::vector<std::string>& GetMessages() const;

    bool HasErrors() const;

  private:
    bool m_validationEnabled{false};
    std::vector<std::string> m_messages;
};

} // namespace Render
