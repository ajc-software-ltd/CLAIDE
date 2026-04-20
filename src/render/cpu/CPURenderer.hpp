#pragma once

#include <string>

namespace Render {

class CPURenderer
{
  public:
    void SetStatus(std::string status);
    const std::string& GetStatus() const;

  private:
    std::string m_status{"CPU fallback renderer idle"};
};

} // namespace Render
