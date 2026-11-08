#pragma once
#include <array>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include "dovah/types/bare_localized_string_id.h"
#include "dovah/utils/localization/data_by_string_category.h"
#include "./category.h"
#include "./language.h"
namespace dovah {
   namespace bsa {
      class archive_collection;
   }
   namespace tes {
      class file;
   }
   class asset_file_buffer;
}

namespace dovah::localization {
   //
   // A counterpart to `string_file_bundle` which allows modification of its contents; suitable 
   // for holding strings loaded for the active file, so that active file localized strings can 
   // be edited.
   //
   class active_string_file_bundle {
      public:
         using const_string_view = std::basic_string_view<const char>;

      protected:
         using id_to_string_map = std::map<bare_localized_string_id, std::string>;
         using id_set           = std::set<bare_localized_string_id>;

         struct string_file {
            id_to_string_map strings_by_id;
         };

         using per_language_data = utils::localization::data_by_string_category<string_file>;

      public:
         active_string_file_bundle(const tes::file&);

         constexpr bool empty() const noexcept;

         template<typename Callable>
            requires std::is_invocable_v<Callable, bare_localized_string_id>
         void for_each_string_id_of_category(category, Callable&&) const;

         template<typename Callable>
            requires std::is_invocable_v<Callable, bare_localized_string_id, const std::string&>
         void for_each_string_matching_criteria(category, language, Callable&&) const;

         constexpr const tes::file& get_tes_file() const noexcept {
            return this->_tes_file;
         }

         bool id_exists(category, bare_localized_string_id) const noexcept;

         void load(const dovah::bsa::archive_collection&);

         bare_localized_string_id create_new_string(category, language, const std::string&);
         std::optional<const_string_view> lookup(category, language, bare_localized_string_id) const noexcept;
         void replace(category, language, bare_localized_string_id, const std::string& new_text);

      protected:
         const tes::file& _tes_file;
         std::array<per_language_data, language_count>    _by_language;
         utils::localization::data_by_string_category<id_set> _defined_ids;

         void _parse_single_file(asset_file_buffer&, category, string_file&, id_set&);
   };
}

#include "./active_string_file_bundle.inl"