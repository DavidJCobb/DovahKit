#include "./path_to_hashes.h"
#include "./split_path.h"

namespace dovah::bsa::utils {
   extern std::optional<hash_pair> path_to_hashes(const std::string_view path_and_name) {
      if (path_and_name.empty())
         return {};

      auto [folder_name, file_name] = split_path(path_and_name);
      if (folder_name.empty() || file_name.empty())
         return {}; // as of Oblivion, Bethesda's code can't hash empty strings
      
      return path_to_hashes(folder_name, file_name);
   }

   extern std::optional<hash_pair> path_to_hashes(const std::string& folder_name, const std::string& file_name) {
      return path_to_hashes(std::string_view(folder_name), std::string_view(file_name));
   }

   extern std::optional<hash_pair> path_to_hashes(const std::string_view& folder_name, const std::string_view& file_name) {
      std::string_view extension;
      std::string_view bare_name = file_name;
      size_t           ext_index = file_name.find_last_of('.');
      if (ext_index != std::string::npos) {
         extension = file_name.substr(ext_index);
         bare_name = file_name.substr(0, ext_index);
         if (bare_name.empty())
            return {}; // as of Oblivion, Bethesda's code can't hash empty strings
      }
      bs_hash folder = bs_hash(folder_name, {});
      bs_hash file   = bs_hash(bare_name,   extension);
      return hash_pair{ folder, file };
   }
}