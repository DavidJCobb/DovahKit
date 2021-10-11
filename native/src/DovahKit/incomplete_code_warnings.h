#pragma once

namespace incomplete_code_warnings {
   inline constexpr const bool is_in_debug = false
      #if defined(_DEBUG) and _DEBUG
         || true
      #endif
   ;

   // Set to false to disallow compiling the program in Release if any loaded-form classes are half-implemented.
   inline constexpr const bool allow_compiling_despite_incomplete_forms = true || is_in_debug;

   // Set to false to disallow compiling the program in Release if any Lua script APIs are half-implemented.
   inline constexpr const bool allow_compiling_despite_incomplete_script_apis = true || is_in_debug;

   // Set to false to disallow compiling the program in Release if any form-editing dialogs are half-implemented.
   inline constexpr const bool allow_compiling_despite_incomplete_form_dialogs = true || is_in_debug;
}