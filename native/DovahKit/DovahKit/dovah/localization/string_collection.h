#pragma once
#include <optional>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "lu/passkey.h"
#include "../types/bare_localized_string_id.h"
#include "../utils/localization/data_by_string_category.h"
#include "./category.h"
#include "./language.h"
namespace dovah {
   namespace bsa {
      class archive_collection;
   }
   namespace localization {
      class  active_string_file_bundle;
      class  string_file_bundle;
      struct use_node;
   }
   namespace tes {
      class file;
   }
   class active_load_order;
   class form_stub;
   class loaded_game_setting;
}

namespace dovah::localization {
   class string_collection {
      public:
         using const_string_view = std::basic_string_view<const char>;

         ~string_collection();

      protected:
         std::vector<string_file_bundle*> _non_active_files;      // owned
         active_string_file_bundle*       _active_file = nullptr; // owned
         std::unordered_map<
            const tes::file*,
            utils::localization::data_by_string_category<
               std::vector<use_node*> // owned
            >
         > _use_info;

      public:
         const active_string_file_bundle* bundle_for_active_file() const;
         const string_file_bundle* bundle_for_non_active_file(const tes::file&) const;
         std::optional<const_string_view> lookup(const tes::file&, category, language, bare_localized_string_id) const noexcept;

      protected:
         void _clear();
         void _destroy_use_node_if_no_users(use_node&);

      public: // passkeyed
         void _clear(lu::passkey<active_load_order>);
         void _load(lu::passkey<active_load_order>, const dovah::bsa::archive_collection&, std::span<const tes::file* const>, const tes::file* active_file);

         // during load or post-load
         use_node& _get_or_create_use_node(
            lu::passkey<form_stub, loaded_game_setting>,
            const tes::file&,
            category,
            bare_localized_string_id
         );

         // post-load
         void _remove_active_file_string_user(lu::passkey<form_stub>, use_node&, form_stub&); // use-node reference may become invalid after this function exits
         void _remove_active_file_string_user(lu::passkey<loaded_game_setting>, use_node&, loaded_game_setting&); // use-node reference may become invalid after this function exits
   };
}