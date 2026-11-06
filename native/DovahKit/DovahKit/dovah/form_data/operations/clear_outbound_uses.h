#pragma once
#include "lu/concepts/instantiation_of/std_array.h"
#include "../helpers/is_or_contains_managed_use.h"
#include "../helpers/is_visitable.h"
#include "dovah/form_stub.h"
#include "dovah/types/managed_form_use.h"
#include "dovah/types/managed_localized_string_use.h"
#include "lu/macros/FORCE_INLINE.define.h"

namespace dovah::form_data_operations {
   #pragma region Forward declarations
      // Form uses.
      template<typename T>
         requires std::is_base_of_v<managed_form_use, T>
      LU_FORCE_INLINE void clear_outbound_uses(form_stub& containing_form, T& data);

      // String uses.
      template<typename T>
         requires std::is_base_of_v<_base_managed_localized_string_use, T>
      LU_FORCE_INLINE void clear_outbound_uses(form_stub& containing_form, T& data);

      // Arrays.
      template<typename T>
         requires (lu::concepts::instantiation_of_std_array<T>&& form_data_helpers::is_or_contains_managed_use<T>)
      LU_FORCE_INLINE void clear_outbound_uses(form_stub& containing_form, T& data);

      // Visitables.
      template<form_data_helpers::is_visitable T>
      void clear_outbound_uses(form_stub& containing_form, T& data);
   #pragma endregion

   // Form uses.
   template<typename T>
      requires std::is_base_of_v<managed_form_use, T>
   LU_FORCE_INLINE void clear_outbound_uses(form_stub& containing_form, T& data) {
      data.set(containing_form, nullptr);
   }

   // String uses.
   template<typename T>
      requires std::is_base_of_v<_base_managed_localized_string_use, T>
   LU_FORCE_INLINE void clear_outbound_uses(form_stub& containing_form, T& data) {
      if (data.is_localized())
         data.set_localized_id(containing_form, 0);
   }

   // Arrays.
   template<typename T>
      requires (lu::concepts::instantiation_of_std_array<T> && form_data_helpers::is_or_contains_managed_use<T>)
   LU_FORCE_INLINE void clear_outbound_uses(form_stub& containing_form, T& data) {
      using value_type = T::value_type;
      for (auto& item : data) {
         if constexpr (std::is_pointer_v<value_type>) { // TODO: doesn't handle `std::unique_ptr`
            if (item)
               clear_outbound_uses(containing_form, *item);
         } else {
            clear_outbound_uses(containing_form, item);
         }
      }
   }

   // Visitables.
   template<form_data_helpers::is_visitable T>
   void clear_outbound_uses(form_stub& containing_form, T& data) {
      T::visit_fields(data, [&containing_form](auto& field) {
         using field_type = std::decay_t<decltype(field)>;
         if constexpr (form_data_helpers::is_or_contains_managed_use<field_type>) {
            clear_outbound_uses(containing_form, field);
         }
      });
   }
}

#include "lu/macros/FORCE_INLINE.undef.h"