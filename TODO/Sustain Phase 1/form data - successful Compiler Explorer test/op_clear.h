#pragma once
#include "lu_std_array.h"
#include "form_data_params.h"

namespace dovah::form_data_ops {
   template<form_data_type Form, uses_form_data_params T>
   void clear_managed_data(Form& form, T& subject) {
      T::visit_fields(subject, [&form](auto& field) {
         using field_type = std::decay_t<decltype(field)>;
         if constexpr (uses_form_data_params<field_type>) {
            clear_managed_data(form, field);
         } else if constexpr (std::is_base_of_v<managed_form_use, field_type>) {
            field.set(form, nullptr);
         } else if constexpr (lu::std_array<field_type>) {
            if constexpr (uses_form_data_params<typename field_type::value_type>) {
               for(auto& item : field)
                  clear_managed_data(form, item);
            }
         }
         //
         // the final/"real" code would also want to handle std::vectors of form uses, 
         // std::vectors of structs that use form data params, et cetera.
         //
      });
   }
   
   template<form_data_type Form>
   void clear_managed_data(Form& form) {
      clear_managed_data(form, form);
   }
}