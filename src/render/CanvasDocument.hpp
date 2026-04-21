#pragma once

#include <string>

namespace Render {

enum class CanvasContentType { None, Text, Image, Video, Model };

class CanvasDocument
{
  public:
    void SetContentType(CanvasContentType type);
    CanvasContentType GetContentType() const;

    void SetSource(std::string source);
    const std::string& GetSource() const;

  private:
    CanvasContentType m_contentType{CanvasContentType::None};
    std::string m_source;
};

} // namespace Render
