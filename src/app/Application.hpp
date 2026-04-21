// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        Application.hpp
// Project:     CLAIDE
// Copyright:   \u00A9 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/app.h>

namespace App {

class Application : public wxApp
{
  public:
    bool OnInit() override;
    int OnExit() override;
};

} // namespace App
