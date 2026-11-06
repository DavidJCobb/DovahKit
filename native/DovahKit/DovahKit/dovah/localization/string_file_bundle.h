#pragma once
#include <array>
#include <map>
#include <optional>
#include <set>
#include <string_view>
#include "dovah/types/asset_file_buffer.h"
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
}

namespace dovah::localization {
   //
   // A read-only holder for loaded localized string content, suitable for use for TES files 
   // besides the active file. See `active_string_file_bundle` for an active-file counterpart.
   //
   class string_file_bundle {
      public:
         using const_string_view = std::basic_string_view<const char>;

      protected:
         struct string_info {
            uint32_t offset; // within `string_file::buffer` i.e. the string file
            uint32_t size;
         };

         using id_to_string_map = std::map<bare_localized_string_id, string_info>;
         using id_set           = std::set<bare_localized_string_id>;

         struct string_file {
            asset_file_buffer buffer;
            id_to_string_map  strings_by_id;
         };

         using per_language_data = utils::localization::data_by_string_category<string_file>;

      public:
         string_file_bundle(const tes::file&);

         template<typename Callable>
            requires std::is_invocable_v<Callable, bare_localized_string_id>
         void for_each_string_id_of_category(category, Callable&&) const;

         // [11/6/2026] MSVC emits spurious C2244 on this forward declaration if `const_string_view` isn't qualified 
         //             in this `requires` clause, because the definition has to qualify it due to the definition's 
         //             differing scope, and qualifying it differently between the declaration and definition will 
         //             make MSVC incorrectly think the declaration and definition signatures differ from each other.
         template<typename Callable>
            requires std::is_invocable_v<Callable, bare_localized_string_id, string_file_bundle::const_string_view>
         void for_each_string_matching_criteria(category, language, Callable&&) const;

         constexpr const tes::file& get_tes_file() const noexcept {
            return this->_tes_file;
         }

         bool id_exists(category, bare_localized_string_id) const noexcept;

         void load(const dovah::bsa::archive_collection&);
         std::optional<const_string_view> lookup(category, language, bare_localized_string_id) const noexcept;

      protected:
         const tes::file& _tes_file;
         std::array<per_language_data, language_count>    _by_language;
         utils::localization::data_by_string_category<id_set> _defined_ids;

         void _parse_single_file(category, string_file&, id_set&);
   };
}

#include "./string_file_bundle.inl"