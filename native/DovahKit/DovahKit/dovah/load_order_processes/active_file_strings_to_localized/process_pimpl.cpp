#include "./process_pimpl.h"
#include <cassert>
#include "dovah/active_load_order.h"
#include "dovah/form_stub.h"
#include "dovah/loaded_game_setting.h"
#include "dovah/localization/active_string_file_bundle.h"
#include "dovah/localization/string_collection.h"
#include "dovah/localization/use_node.h"

namespace dovah::load_order_processes::active_file_strings_to_localized {
   process_pimpl::process_pimpl(lu::passkey<process>, const process_params& params) : params(params) {}

   void process_pimpl::convert_localized_string(
      string_to_id_maps& strings_to_ids,
      form_stub& containing_form,
      localization::category category,
      _base_managed_localized_string_use& use
   ) {
      auto& active_file = *this->params.active_load_order.get_active_file();
      auto& lsc         = this->params.active_load_order.get_localized_string_collection();
      if (use.empty()) {
         use._force_localized_id({}, active_file, 0);
      } else {
         assert(!use.is_localized());
         std::string s = use._take_non_localized_string({});
         bare_localized_string_id id = 0;

         auto& map = strings_to_ids.for_category(category);
         auto  mit = map.find(s);
         if (mit == map.end()) {
            lsc._get_active_strings_as_mutable({})._create_id_and_store_string({}, category, this->params.languages, s);
            map[s] = id;
         } else {
            id = mit->second;
         }
         if (id) {
            auto& use_info = lsc._get_or_create_use_node({}, active_file, category, id);
            containing_form._on_string_converted_to_localized({}, use_info);
         }
         use._force_localized_id({}, active_file, id);
      }
   }

   void process_pimpl::convert_localized_string(
      string_to_id_maps& strings_to_ids,
      loaded_game_setting& gmst
   ) {
      const auto& prior_value  = gmst.get_value_variant();
      assert(std::holds_alternative<loaded_game_setting::string_value_type>(prior_value) && "This must be a string!");
      const auto& prior_string = std::get<loaded_game_setting::string_value_type>(prior_value);
      assert(std::holds_alternative<std::string>(prior_string) && "This string must not already be localized!");

      auto& active_file = *this->params.active_load_order.get_active_file();
      auto& lsc         = this->params.active_load_order.get_localized_string_collection();

      auto s = std::get<std::string>(prior_string);
      bare_localized_string_id id = 0;
      if (!s.empty()) {
         auto& map = strings_to_ids.for_category(localization::category::general);
         auto  mit = map.find(s);
         if (mit == map.end()) {
            lsc._get_active_strings_as_mutable({})._create_id_and_store_string({}, localization::category::general, this->params.languages, s);
            map[s] = id;
         } else {
            id = mit->second;
         }
      }
      localization::use_node* use_node = nullptr;
      if (id) {
         use_node = &lsc._get_or_create_use_node({}, active_file, localization::category::general, id);
      }
      gmst._on_string_converted_to_localized({}, use_node);
   }
}