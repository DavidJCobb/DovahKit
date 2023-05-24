#pragma once
#define VK_USE_PLATFORM_WIN32_KHR

// Winbase triggers portability warnings when using MSVC's standards-conforming C(++) preprocessor
#pragma warning(disable: 5105)

#include <vulkan/vulkan.h> // includes Windows.h :(
#include "helpers/windows.h"

#pragma warning(default: 5105)