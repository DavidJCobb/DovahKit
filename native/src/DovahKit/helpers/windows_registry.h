#pragma once
#include <string>

namespace cobb {
   namespace windows_registry {
      #if UNICODE
         using char_t    = wchar_t;
         using string_t  = std::wstring;
      #else
         using char_t    = char;
         using string_t  = std::string;
      #endif
      using cstring_t = const char_t*;

      enum class hkey {
         classes_root,
         current_config,
         current_user,
         local_machine,
         performance_data,
         performance_nlstext,
         performance_text,
         users,
      };
      struct key_handle {
         key_handle();
         ~key_handle();
      };
      //
      bool get_string_value(hkey, cstring_t key, cstring_t value, string_t& out);
   }
}