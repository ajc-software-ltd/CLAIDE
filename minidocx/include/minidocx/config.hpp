// ============================================================================
// MINIDOCX
// ============================================================================
// File:        config.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#ifndef MINIDOCX_NAMESPACE
#define MINIDOCX_NAMESPACE md
#endif

#ifdef WIN32
# ifdef MINIDOCX_SHARED
#   ifdef MINIDOCX_EXPORTS
#     define MINIDOCX_API __declspec(dllexport)
#   else
#     define MINIDOCX_API __declspec(dllimport)
#   endif
# else
#  define MINIDOCX_API
# endif
#else
# define MINIDOCX_API
#endif
