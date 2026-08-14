#pragma once
#include <string>
#include <string_view>
#include <utility> // std::pair
#include "dovah/bs_hash.h"

namespace dovah::bsa::utils {
   // Split a path into its "folder" and "file" parts.
   extern std::pair<std::string, std::string> split_path(const std::string_view);

   // Split an already case-folded path, which uses only preferred path 
   // separators, into its "folder" and "file" parts.
   extern std::pair<std::string, std::string> split_folded_path(const std::string_view);
}