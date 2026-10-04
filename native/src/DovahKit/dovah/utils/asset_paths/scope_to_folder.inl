#pragma once
#include "./scope_to_folder.h"
#include "helpers/string/strieq_ascii.h"
#include "helpers/string/strifind_ascii.h"

namespace dovah::utils::asset_paths {
   namespace impl::scope_to_folder {
      //
      // If the folder name we want to search for contains only one occurrence of its first 
      // character, then upon finding a failed match, we can skip ahead by the full length 
      // of the name.
      // 
      // For example, if we're searching for folder name "foo", and we have "foobar/0/foo/", 
      // we'll find the first "foo". It's not followed by a directory separator, so we can 
      // skip ahead by three characters.
      // 
      // On the other hand, if we're searching for "sos" and we have "sosos/", we'll find 
      // the first "sos". It's not followed by a directory separator, but if we skip ahead 
      // by 3, then we'll miss the instance that *is* followed by a separator.
      //
      constexpr bool folder_name_candidates_may_overlap(const std::string_view folder_name) {
         if (folder_name.empty())
            return true;
         char c = folder_name[0];
         for (size_t i = 1; i < folder_name.size(); ++i)
            if (folder_name[i] == c)
               return true;
         return false;
      }
   }

   template<const cobb::cs folder_name_cs, impl::constructible_from_string_like Out>
   extern Out scope_to_folder(const std::string_view path) {
      constexpr const auto folder_name = std::string_view(folder_name_cs.data(), folder_name_cs.size());
      constexpr bool folder_name_candidates_may_overlap = impl::scope_to_folder::folder_name_candidates_may_overlap(folder_name);

      const auto size = path.size();
      if (size > folder_name.size()) {
         const auto possible_leading_segment = std::string_view(path.data(), folder_name.size());
         if (cobb::strieq_ascii(possible_leading_segment, folder_name)) {
            //
            // Edge-case: for the leading path segment, the game only checks for '\\'.
            //
            if (path[folder_name.size()] == '\\') {
               return path; // already prefixed
            }
         }
            
         auto i = cobb::strifind_ascii(path, folder_name);
         while (i != std::string::npos) {
            if (i + folder_name.size() + 1 >= size) {
               break;
            }
            char c = path[i + folder_name.size()];
            if (c == '/' || c == '\\') {
               return std::string_view(path).substr(i);
            }
            i = cobb::strifind_ascii(path, folder_name, folder_name_candidates_may_overlap ? i + 1 : i + folder_name.size());
         }
      }

      std::string prefixed;
      prefixed.reserve(folder_name.size() + 1 + path.size());
      prefixed += folder_name;
      prefixed += '\\';
      prefixed += path;
      return prefixed;
   }
}
