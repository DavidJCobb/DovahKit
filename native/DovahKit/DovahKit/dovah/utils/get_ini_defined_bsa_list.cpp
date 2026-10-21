#include "./get_ini_defined_bsa_list.h"
#include "lu/ascii/trim.h"
#include "lu/windows/read_single_ini_setting.h"
#include <windows.h>
#include <Shlobj.h>

namespace dovah::utils {
   std::vector<std::filesystem::path> get_ini_defined_bsa_list(game which_game) {
      std::vector<std::filesystem::path> out;
      
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
      filepath += L"\\My Games\\";
      switch (which_game) {
         case game::skyrim_classic:
            filepath += L"Skyrim\\";
            break;
         case game::skyrim_special:
            filepath += L"Skyrim Special Edition\\";
            break;
         default:
            return out;
      }
      filepath += L"Skyrim.INI";
      
      std::wstring raw = lu::windows::read_single_ini_setting(filepath.c_str(), L"Archive", L"sResourceArchiveList");
      size_t pos;
      while ((pos = raw.find(',')) != std::string::npos) {
         out.push_back(lu::ascii::trim(raw.substr(0, pos)));
         raw.erase(0, pos + 1);
      }
      if (!raw.empty())
         out.push_back(lu::ascii::trim(raw));
      
      raw = lu::windows::read_single_ini_setting(filepath.c_str(), L"Archive", L"sResourceArchiveList2");
      while ((pos = raw.find(',')) != std::string::npos) {
         out.push_back(lu::ascii::trim(raw.substr(0, pos)));
         raw.erase(0, pos + 1);
      }
      if (!raw.empty())
         out.push_back(lu::ascii::trim(raw));
      
      return out;
   }
}