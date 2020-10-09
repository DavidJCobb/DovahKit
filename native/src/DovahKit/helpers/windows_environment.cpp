#include "windows_environment.h"
#include "windows.h"

namespace cobb::windows {
   void expand_environment_variables(std::string& original) {
      std::string out;
      out.resize(256);
      DWORD count = ExpandEnvironmentStringsA(original.c_str(), out.data(), out.size());
      while (count > out.size()) {
         out.resize(out.size() + 256);
         count = ExpandEnvironmentStringsA(original.c_str(), out.data(), out.size());
      }
      auto pos = out.find_last_not_of('\0');
      out.resize(pos == std::string::npos ? 0 : pos + 1);
      std::swap(original, out);
   }
   void expand_environment_variables(std::wstring& original) {
      std::wstring out;
      out.resize(256);
      DWORD count = ExpandEnvironmentStringsW(original.c_str(), out.data(), out.size());
      while (count > out.size()) {
         out.resize(out.size() + 256);
         count = ExpandEnvironmentStringsW(original.c_str(), out.data(), out.size());
      }
      auto pos = out.find_last_not_of(L'\0');
      out.resize(pos == std::wstring::npos ? 0 : pos + 1);
      std::swap(original, out);
   }
}