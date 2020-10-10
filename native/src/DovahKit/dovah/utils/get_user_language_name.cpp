#include "get_user_language_name.h"
#include "../../helpers/strings.h"
#include "../../helpers/windows_ini.h"
#include "Shlobj.h"

namespace dovah::utils {
   std::string get_user_language_name() {
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
         return "";
      filepath += L"\\My Games\\Skyrim\\Skyrim.INI";
      //
      auto wide = cobb::read_single_ini_string_setting(filepath.c_str(), L"General", L"sLanguage");
      if (wide.empty())
         return "ENGLISH";
      std::string language;
      for (auto c : wide)
         language += c;
      return language;
   }
}