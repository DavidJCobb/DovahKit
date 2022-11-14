/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#pragma once
#include "windows_registry.h"
#include <string>
#include "./windows.h"

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

   void _shrink_to_null(string_t& out) {
      auto i = out.find(TCHAR('\0'));
      if (i == std::string::npos)
         return;
      out.resize(i);
   }
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
   bool get_string_value(hkey h, const TCHAR* subkey, const TCHAR* value, string_t& out) {
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
         _shrink_to_null(out);
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
         if (result == ERROR_SUCCESS) {
            RegCloseKey(handle);
            _shrink_to_null(out);
            return true;
         }
      }
      RegCloseKey(handle);
      out.clear();
      return false;
   }
}