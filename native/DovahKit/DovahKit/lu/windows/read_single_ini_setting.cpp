#include "./read_single_ini_setting.h"
#include <windows.h>

namespace {
   constexpr const size_t base_size_to_try      = 256;
   constexpr const size_t size_increment_to_try = 256;
}

namespace lu::windows {
   std::string read_single_ini_setting(const std::filesystem::path& file, const std::string& category, const std::string& key) {
      const std::string fileA = file.string();
      std::string out;
      out.resize(base_size_to_try);
      DWORD count = GetPrivateProfileStringA(category.c_str(), key.c_str(), nullptr, out.data(), (DWORD)out.size(), fileA.c_str());
      while (count == out.size() - 1) {
         out.resize(out.size() + size_increment_to_try);
         count = GetPrivateProfileStringA(category.c_str(), key.c_str(), nullptr, out.data(), (DWORD)out.size(), fileA.c_str());
      }
      out.resize(count);
      return out;
   }

   std::wstring read_single_ini_setting(const std::filesystem::path& file, const std::wstring& category, const std::wstring& key) {
      const std::wstring fileW = file.wstring();
      std::wstring out;
      out.resize(base_size_to_try);
      DWORD count = GetPrivateProfileStringW(category.c_str(), key.c_str(), nullptr, out.data(), (DWORD)out.size(), fileW.c_str());
      while (count == out.size() - 1) {
         out.resize(out.size() + size_increment_to_try);
         count = GetPrivateProfileStringW(category.c_str(), key.c_str(), nullptr, out.data(), (DWORD)out.size(), fileW.c_str());
      }
      out.resize(count);
      return out;
   }
}