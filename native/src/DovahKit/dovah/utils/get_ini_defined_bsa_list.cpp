#include "get_ini_defined_bsa_list.h"
#include "../../helpers/strings.h"
#include "../../helpers/windows_ini.h"
#include "Shlobj.h"

namespace dovah::utils {
   std::vector<std::filesystem::path> get_ini_defined_bsa_list() {
      std::vector<std::filesystem::path> out;
      //
      std::wstring filepath;
      {
         PWSTR string = nullptr;
         auto  result = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &string);
         if (result == S_OK && string) {
            filepath = string;
            CoTaskMemFree(string);
         }
      }
      if (filepath.empty())
         return out;
      filepath += L"\\My Games\\Skyrim\\Skyrim.INI";
      //
      std::wstring raw = cobb::read_single_ini_string_setting(filepath.c_str(), L"Archive", L"sResourceArchiveList");
      size_t pos;
      while ((pos = raw.find(',')) != std::string::npos) {
         out.push_back(cobb::trim(raw.substr(0, pos)));
         raw.erase(0, pos + 1);
      }
      if (!raw.empty())
         out.push_back(cobb::trim(raw));
      //
      raw = cobb::read_single_ini_string_setting(filepath.c_str(), L"Archive", L"sResourceArchiveList2");
      while ((pos = raw.find(',')) != std::string::npos) {
         out.push_back(cobb::trim(raw.substr(0, pos)));
         raw.erase(0, pos + 1);
      }
      if (!raw.empty())
         out.push_back(cobb::trim(raw));
      //
      return out;
   }
}