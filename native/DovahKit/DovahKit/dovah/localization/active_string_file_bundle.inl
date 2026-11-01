#pragma once
#include "./active_string_file_bundle.h"
#include "lu/foreach_lambda_result.h"

namespace dovah::localization {
   template<typename Callable>
      requires std::is_invocable_v<Callable, bare_localized_string_id>
   void active_string_file_bundle::for_each_string_id_of_type(localized_string_type type, Callable&& callable) const {
      const auto& presence = this->_defined_ids.for_type(type);
      for (const auto id : presence) {
         if constexpr (std::is_invocable_r_v<lu::foreach_lambda_result, Callable, bare_localized_string_id>) {
            auto result = callable(id);
            if (result == lu::foreach_lambda_result::stop)
               break;
         } else {
            callable(id);
         }
      }
   }

   template<typename Callable>
      requires std::is_invocable_v<Callable, bare_localized_string_id, const std::string&>
   void active_string_file_bundle::for_each_string_matching_criteria(localized_string_type type, language lang, Callable&& callable) const {
      const auto& file = this->_by_language[(size_t)lang].for_type(type);
      for (const auto& [id, string] : file.strings_by_id) {
         if constexpr (std::is_invocable_r_v<lu::foreach_lambda_result, Callable, bare_localized_string_id, const std::string&>) {
            auto result = callable(id, string);
            if (result == lu::foreach_lambda_result::stop)
               break;
         } else {
            callable(id, string);
         }
      }
   }
}