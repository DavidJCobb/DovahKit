#pragma once
#include <string>
#include <string_view>
#include <optional>
#include <utility> // std::string
#include "dovah/bs_hash.h"

namespace dovah::bsa::utils {
   using hash_pair = std::pair<bs_hash, bs_hash>;

   // Convert a path into BS-hashes of the "folder" and "file" parts.
   extern std::optional<hash_pair> path_to_hashes(const std::string_view full_path);
   extern std::optional<hash_pair> path_to_hashes(const std::string& folder, const std::string& file);
   extern std::optional<hash_pair> path_to_hashes(const std::string_view& folder, const std::string_view& file);
}