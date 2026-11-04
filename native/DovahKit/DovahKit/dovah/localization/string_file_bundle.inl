#pragma once
#include "./string_file_bundle.h"
#include "lu/foreach_lambda_result.h"

namespace dovah::localization {
   template<typename Callable>
      requires std::is_invocable_v<Callable, bare_localized_string_id>
   void string_file_bundle::for_each_string_id_of_category(category category, Callable&& callable) const {
      const auto& presence = this->_defined_ids.for_category(category);
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
      requires std::is_invocable_v<Callable, bare_localized_string_id, string_file_bundle::const_string_view>
   void string_file_bundle::for_each_string_matching_criteria(category category, language lang, Callable&& callable) const {
      const auto& file = this->_by_language[(size_t)lang].for_category(category);
      for (const auto& [id, info] : file.strings_by_id) {
         const auto string = const_string_view(
            (const char*)file.buffer.data_at(info.offset),
            info.size
         );
         if constexpr (std::is_invocable_r_v<lu::foreach_lambda_result, Callable, bare_localized_string_id, const_string_view>) {
            auto result = callable(id, string);
            if (result == lu::foreach_lambda_result::stop)
               break;
         } else {
            callable(id, string);
         }
      }
   }
}