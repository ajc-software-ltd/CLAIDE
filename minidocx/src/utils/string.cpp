// ============================================================================
// MINIDOCX
// ============================================================================
// File:        string.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#include "utils/string.hpp"

#include <algorithm>
#include <cctype>


namespace MINIDOCX_NAMESPACE
{
  std::string removeSpaces(std::string str) {
    std::string tmp{ std::move(str) };
    tmp.erase(std::remove_if(tmp.begin(), tmp.end(), [](unsigned char ch) { return std::isspace(ch) != 0; }), tmp.end());
    return tmp;
  }
}
