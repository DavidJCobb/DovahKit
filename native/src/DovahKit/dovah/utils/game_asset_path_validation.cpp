#include "game_asset_path_validation.h"
#include <immintrin.h>
#include <string_view>
#include "../../helpers/cpuinfo.h"
#include "../files/bsa/bsa_archive.h"

namespace dovah::utils {
   void game_asset_path_validation::validate() {
      this->errors = 0;
      if (this->path.empty())
         return;
      //
      size_t last_period    = std::string::npos;
      size_t last_separator = std::string::npos;
      if (this->normalize_first) {
         bsa_archive::normalize_path_or_path_component(this->path);
      }
      const auto& path = this->path;
      size_t      size = path.size();
      //
      // Check for non-ASCII characters:
      //
      if (cobb::cpuinfo::get().extension_support.sse_2) {
         bool   fail = false;
         size_t i    = 0;
         auto   base = _mm_set1_epi8(0x7F);
         auto*  data = path.data();
         for (; i + 15 < size; i += 16) {
            //
            // The byte-comparison intrinsics assume signed bytes, which we don't want, so 
            // let's instead try an alternate approach:
            // 
            // for (std::byte c : data) {
            //    auto d = std::max(c, 0x7F);
            //    d = (d == 0x7F) ? 0xFF : 0x00;
            //    if (d != 0xFF) {
            //       fail = true;
            //       break;
            //    }
            // }
            //
            auto c = _mm_loadu_si128((const __m128i*)(data + i));
            auto d = _mm_max_epu8(c, base);
            d = _mm_cmpeq_epi8(d, base);
            if (!_mm_test_all_ones(d)) {
               fail = true;
               break;
            }
         }
         if (!fail) {
            for (; i < size; ++i) {
               if (data[i] > 0x7F) {
                  fail = true;
                  break;
               }
            }
         }
         if (fail)
            this->errors |= error_flag::has_non_ascii_characters;
      } else {
         for (size_t i = 0; i < size; ++i) {
            if (path[i] > 0x7F) {
               this->errors |= error_flag::has_non_ascii_characters;
               break;
            }
         }
      }
      //
      // Check for path structure issues:
      //
      last_period    = path.find_last_of('.');
      last_separator = path.find_last_of('\\');
      if (last_period != std::string::npos) {
         bool has_extension = true;
         if (last_separator != std::string::npos) {
            has_extension = (last_period > last_separator);
            if (!has_extension) {
               this->errors |= error_flag::period_in_folder_name;
            } else if (last_separator > 0) {
               //
               // While we're here, let's also test for periods earlier in the path.
               //
               if (path.find_last_of('.', last_separator - 1) != std::string::npos)
                  this->errors |= error_flag::period_in_folder_name;
            }
         }
         if (has_extension) {
            if (last_period + 10 < size)
               this->errors |= error_flag::extension_too_long;
         }
      }
      if (!this->normalize_first) { // if we normalized, then all path separators should be correct
         if (path.find('/') != std::string::npos)
            this->errors |= error_flag::wrong_path_separators;
      }
      if (this->is_armoraddon_mesh) {
         auto view = std::string_view(path);
         if (last_separator != std::string::npos) {
            size_t start = last_separator + 1;
            size_t count = (last_period != std::string::npos) ? last_period - start : std::string::npos;
            view = view.substr(start, count);
         }
         //
         size_t last_underscore = view.find_last_of('_');
         if (last_underscore != std::string::npos) {
            bool   okay = false;
            size_t size = view.size();
            if (last_underscore + 2 == size) {
               view = view.substr(last_underscore);
               if (view == "_0" || view == "_1")
                  okay = true;
            }
            if (!okay) {
               this->errors |= error_flag::armoraddon_underscore_issue;
               this->errors |= error_flag::armoraddon_suffix_missing;
            }
         } else {
            this->errors |= error_flag::armoraddon_suffix_missing;
         }
      }
      {
         size_t i = 0;
         if (path[0] == '\\')
            ++i;
         if (i + 5 < size) {
            const auto* start = path.data() + i;
            if (_strnicmp(start, "data", 4) == 0) {
               if (path[i + 4] != '\\')
                  this->errors |= error_flag::first_folder_is_data_superstring;
               else if (strncmp(start, "data", 4) != 0 && strncmp(start, "Data", 4) != 0)
                  this->errors |= error_flag::data_prefix_miscapitalized;
            }
         }
      }
   }
}