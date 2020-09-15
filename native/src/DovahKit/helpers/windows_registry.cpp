#pragma once
#include "windows_registry.h"
#include <string>
#include "windows.h"
#include "intrusive_windows_defines.h"

namespace {
   using namespace cobb::windows_registry;

   HKEY _enum_to_win(hkey h) {
      switch (h) {
         case hkey::classes_root: return HKEY_CLASSES_ROOT;
         case hkey::current_config: return HKEY_CURRENT_CONFIG;
         case hkey::current_user: return HKEY_CURRENT_USER;
         case hkey::local_machine: return HKEY_LOCAL_MACHINE;
         case hkey::performance_data: return HKEY_PERFORMANCE_DATA;
         case hkey::performance_nlstext: return HKEY_PERFORMANCE_NLSTEXT;
         case hkey::performance_text: return HKEY_PERFORMANCE_TEXT;
         case hkey::users: return HKEY_USERS;
      }
      return 0;
   }

   class _key_read_handle {
      protected:
         HKEY key;
         bool valid = false;
      public:
         _key_read_handle() {}
         _key_read_handle(hkey h, cstring_t subkey) {
            LRESULT result = RegOpenKeyEx(
               _enum_to_win(h),
               subkey,
               0,
               KEY_QUERY_VALUE | KEY_WOW64_32KEY,
               &this->key
            );
            this->valid = result == ERROR_SUCCESS;
         }
         ~_key_read_handle() {
            if (!valid)
               return;
            RegCloseKey(this->key);
            this->valid = false;
         }
   };
}

namespace cobb::windows_registry {
   namespace {
      HKEY _enum_to_win(hkey h) {
         switch (h) {
            case hkey::classes_root: return HKEY_CLASSES_ROOT;
            case hkey::current_config: return HKEY_CURRENT_CONFIG;
            case hkey::current_user: return HKEY_CURRENT_USER;
            case hkey::local_machine: return HKEY_LOCAL_MACHINE;
            case hkey::performance_data: return HKEY_PERFORMANCE_DATA;
            case hkey::performance_nlstext: return HKEY_PERFORMANCE_NLSTEXT;
            case hkey::performance_text: return HKEY_PERFORMANCE_TEXT;
            case hkey::users: return HKEY_USERS;
         }
         return 0;
      }
      static constexpr hkey _invalid_hkey = (hkey)0x01234567;
   }
   //
   bool get_string_value(hkey h, cstring_t subkey, cstring_t value, std::wstring& out) {
      HKEY    handle;
      LSTATUS result = RegOpenKeyEx(_enum_to_win(h), subkey, 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &handle);
      if (result != ERROR_SUCCESS)
         return false;
      //
      DWORD size = (out.size() + 1) * sizeof(decltype(*out.data())); // this should include the terminating null char, and it should be in bytes
      result = RegGetValue(
         handle, nullptr, value,
         RRF_RT_REG_SZ | RRF_RT_REG_MULTI_SZ | RRF_RT_REG_EXPAND_SZ | KEY_WOW64_32KEY,
         nullptr,
         out.data(),
         &size
      );
      if (result == ERROR_SUCCESS) {
         RegCloseKey(handle);
         return true;
      }
      if (result == ERROR_MORE_DATA) {
         out.resize(size);
         result = RegGetValue(
            handle, nullptr, value,
            RRF_RT_REG_SZ | RRF_RT_REG_MULTI_SZ | RRF_RT_REG_EXPAND_SZ | KEY_WOW64_32KEY,
            nullptr,
            out.data(),
            &size
         );
         RegCloseKey(handle);
         if (result == ERROR_SUCCESS)
            return true;
      }
      RegCloseKey(handle);
      out.clear();
      return false;
   }
}