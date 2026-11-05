#pragma once
#include "lu/concepts/instantiation_of/std_array.h"
#include "lu/concepts/instantiation_of/std_pair.h"
#include "lu/concepts/instantiation_of/std_vector.h"
#include "lu/concepts/map_like_type.h"
#include "lu/concepts/supports_range_based_for_loops.h"
#include "../helpers/is_visitable.h"
#include "dovah/form_stub.h"
#include "dovah/types/managed_form_use.h"
#include "dovah/types/managed_localized_string_use.h"
#include "lu/macros/FORCE_INLINE.define.h"

namespace dovah::form_data_operations {
   template<typename T>
      requires (!std::is_pointer_v<T>)
   LU_FORCE_INLINE void clear_outbound_uses(form_stub& containing_form, T& data) {
   }

   template<typename T>
      requires std::is_base_of_v<managed_form_use, T>
   LU_FORCE_INLINE void clear_outbound_uses<T>(dovah::form_stub& containing_form, T& data) {
      data.set(containing_form, nullptr);
   }

   template<typename T>
      requires std::is_base_of_v<_base_managed_localized_string_use, T>
   LU_FORCE_INLINE void clear_outbound_uses<T>(form_stub& containing_form, T& data) {
      if (data.is_localized())
         data.set_localized_id(containing_form, 0);
   }
   
   // General-purpose handler for std::map/std::unordered_map.
   template<typename T>
      requires (lu::concepts::map_like_type<T>)
   void clear_outbound_uses<T>(dovah::form_stub& containing_form, T& data) {
      using key_type    = T::key_type;
      using mapped_type = T::mapped_type;
      constexpr const bool key_needs_clearing = std::is_same_v<key_type, managed_form_use> || std::is_same_v<key_type, _base_managed_localized_string_use>;
      for (auto& [k, v] : data) {
         if constexpr (key_needs_clearing) {
            //
            // We can't directly modify the key, but we can copy it and clear the copy 
            // to trigger updating of whatever use info it may influence.
            //
            auto dummy = k;
            clear_outbound_uses(containing_form, dummy);
         }
         clear_outbound_uses(containing_form, v);
      }
      if constexpr (key_needs_clearing) {
         data.clear();
      }
   }

   // General-purpose handler for containers.
   template<typename T>
      requires (lu::concepts::supports_range_based_for_loops<T> && !lu::concepts::map_like_type<T>)
   LU_FORCE_INLINE void clear_outbound_uses<T>(dovah::form_stub& containing_form, T& data) {
      using value_type = T::value_type;
      for (auto& item : data) {
         using item_type = std::decay_t<decltype(item)>;
         if constexpr (std::is_pointer_v<item_type>) { // TODO: doesn't handle `std::unique_ptr`
            if (item)
               clear_outbound_uses(containing_form, *item);
         } else {
            clear_outbound_uses(containing_form, item);
         }
      }
      if constexpr (std::is_same_v<value_type, managed_form_use> || std::is_same_v<value_type, _base_managed_localized_string_use>) {
         data.clear();
      }
   }

   template<dovah::form_data_helpers::is_visitable T>
   void clear_outbound_uses(form_stub& containing_form, T& data) {
      T::visit_members(data, [&containing_form](auto& field) {
         clear_outbound_uses(containing_form, field);
      });
   }
}

#include "lu/macros/FORCE_INLINE.undef.h"