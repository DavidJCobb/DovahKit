#pragma once
#include "lu/concepts/instantiation_of/std_array.h"
#include "lu/concepts/instantiation_of/std_pair.h"
#include "lu/concepts/instantiation_of/std_vector.h"
#include "lu/concepts/supports_range_based_for_loops.h"
#include "../helpers/is_visitable.h"
#include "dovah/form_stub.h"
#include "dovah/types/managed_form_use.h"
#include "dovah/types/managed_localized_string_use.h"

namespace dovah::form_data_operations {
   template<typename T>
      requires (!std::is_pointer_v<T>)
   void clear_outbound_uses(dovah::form_stub& containing_form, T& data) {
   }

   template<typename T>
      requires std::is_base_of_v<dovah::managed_form_use, T>
   void clear_outbound_uses<T>(dovah::form_stub& containing_form, T& data) {
      data.set(containing_form, nullptr);
   }

   template<typename T>
      requires std::is_base_of_v<dovah::_base_managed_localized_string_use, T>
   void clear_outbound_uses<T>(dovah::form_stub& containing_form, T& data) {
      if (data.is_localized())
         data.set_localized_id(containing_form, 0);
   }

   // General-purpose handler for containers.
   template<typename T>
      requires (lu::concepts::supports_range_based_for_loops<T>)
   void clear_outbound_uses<T>(dovah::form_stub& containing_form, T& data) {
      for (auto& item : data) {
         using item_type = std::decay_t<decltype(item)>;
         if constexpr (lu::concepts::instantiation_of_std_pair<item_type>) {
            //
            // Special-case handling for `std::pair`, since `data` might be a `std::map`/`std::unordered_map`.
            //
            clear_outbound_uses(containing_form, item.first);

            using second_type = std::tuple_element_t<1, item_type>;
            if constexpr (std::is_pointer_v<second_type>) { // TODO: doesn't handle `std::unique_ptr`; TODO: unify with the logic below
               if (item)
                  clear_outbound_uses(containing_form, *item.second);
            } else {
               clear_outbound_uses(containing_form, item.second);
            }
         } else {
            if constexpr (std::is_pointer_v<item_type>) { // TODO: doesn't handle `std::unique_ptr`; TODO: unify with the logic above
               if (item)
                  clear_outbound_uses(containing_form, *item);
            } else {
               clear_outbound_uses(containing_form, item);
            }
         }
      }
   }

   template<dovah::form_data_helpers::is_visitable T>
   void clear_outbound_uses(form_stub& containing_form, T& data) {
      T::visit_members(data, [&containing_form](auto& field) {
         clear_outbound_uses(containing_form, field);
      });
   }
}