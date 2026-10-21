#pragma once
#include <array>
#include <filesystem>
#include <map>
#include <optional>
#include <string_view>
#include "dovah/types/asset_file_buffer.h"
#include "dovah/types/bare_localized_string_id.h"
#include "dovah/types/localized_string_type.h"
#include "./language.h"
namespace dovah::bsa {
   class archive_collection;
}

namespace dovah::localization {
   class string_file_bundle {
      public:
         using const_string_view = std::basic_string_view<const char>;

      protected:
         using id_to_string_map = std::map<bare_localized_string_id, std::string_view>;

         struct string_file {
            asset_file_buffer buffer;
            id_to_string_map  strings_by_id;
         };

         struct per_language_data {
            string_file description;
            string_file general;
            string_file info;
         };

      public:
         string_file_bundle(const std::string& tes_filename);

         void load(const dovah::bsa::archive_collection&);
         std::optional<const_string_view> lookup(localized_string_type, language, bare_localized_string_id) const noexcept;

      protected:
         std::string _tes_filename;
         std::array<per_language_data, language_count> _by_language;

         void _parse_single_file(localized_string_type, string_file&);
   };
}