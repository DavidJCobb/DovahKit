#include "./split_path.h"
#include "../constants/path_separators.h"

namespace dovah::bsa::utils {
   extern std::pair<std::string, std::string> split_path(const std::string_view full_path) {
      std::pair<std::string, std::string> result;
      auto& [out_folder, out_file] = result;

      size_t size = full_path.size();
      for (char c : full_path) {
         if (c == preferred_path_separator || c == secondary_path_separator) {
            if (out_file.empty()) // skip leading and redundant slashes
               continue;
            if (!out_folder.empty())
               out_folder += preferred_path_separator;
            out_folder += out_file;
            out_file.clear();
         } else {
            if (c >= 'A' && c <= 'Z')
               c += 0x20;
            out_file += c;
         }
      }

      return result;
   }

   extern std::pair<std::string, std::string> split_folded_path(const std::string_view full_path) {
      std::pair<std::string, std::string> result;
      auto& [out_folder, out_file] = result;

      size_t size = full_path.size();
      for (char c : full_path) {
         if (c == preferred_path_separator) {
            if (out_file.empty()) // skip leading and redundant slashes
               continue;
            if (!out_folder.empty())
               out_folder += preferred_path_separator;
            out_folder += out_file;
            out_file.clear();
         } else {
            out_file += c;
         }
      }

      return result;
   }
}