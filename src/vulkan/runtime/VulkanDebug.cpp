#include "vulkan/runtime/VulkanDebug.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace Render {

void VulkanDebug::EnableValidation(bool enabled) {
    m_validationEnabled = enabled;
}

bool VulkanDebug::IsValidationEnabled() const {
    return m_validationEnabled;
}

void VulkanDebug::AddMessage(std::string message) {
    m_messages.push_back(std::move(message));
}

const std::vector<std::string>& VulkanDebug::GetMessages() const {
    return m_messages;
}

bool VulkanDebug::HasErrors() const {
    return std::any_of(m_messages.begin(), m_messages.end(), [](const std::string& message) {
        std::string lowered = message;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return lowered.find("error") != std::string::npos;
    });
}

} // namespace Render
