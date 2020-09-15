#pragma once
#include <string>

namespace cobb {
   namespace windows_registry {
      #if UNICODE
         using cstring_t = const wchar_t*;
      #else
         using cstring_t = const char*;
      #endif

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
      bool get_string_value(hkey, cstring_t key, cstring_t value, std::wstring& out);
   }
}