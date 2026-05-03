#pragma once
#include <tuple>
#include "lu_std_array.h"
#include "form_data_params.h"
#include "form_data.h"

namespace dovah::form_data_ops {
   namespace impl {
      template<typename T>
      concept form_stub_pointer_type = (std::is_pointer_v<T> && std::is_base_of_v<form_stub, std::remove_pointer_t<T>>);

      template<typename T>
      concept do_member_wise_assign = (
         uses_form_data_params<T>
      || (std::is_base_of_v<managed_form_use, T> || form_stub_pointer_type<T>)
      // || ...anything else that needs special handling...
      );

      template<form_data_type Form, typename FieldL, typename FieldR>
      void _assign_field(Form& lhs_top, FieldL& lhs_field, const FieldR& rhs_field) {
         auto _visit_members = [&lhs_top](auto& l, auto& r) {
            _assign_field(lhs_top, l, r);
         };

         if constexpr (uses_form_data_params<FieldL>) {
            FieldL::visit_fields_in_tandem(lhs_field, rhs_field, _visit_members);
         } else if constexpr (std::is_base_of_v<managed_form_use, FieldL> || form_stub_pointer_type<FieldL>) {
            dovah::form_stub* v = nullptr;
            if constexpr (std::is_base_of_v<managed_form_use, FieldR>) {
               v = rhs_field.get();
            } else {
               static_assert(form_stub_pointer_type<FieldR>);
               v = rhs_field;
            }
            if constexpr (std::is_base_of_v<managed_form_use, FieldL>) {
               lhs_field.set(lhs_top, v);
            } else {
               static_assert(form_stub_pointer_type<FieldL>);
               lhs_field = v;
            }
         } else if constexpr (lu::std_array<FieldL>) {
            static_assert(
               lu::std_array<FieldR> && std::tuple_size_v<FieldL> == std::tuple_size_v<FieldR>,
               "Field types must match: if LHS is an array, RHS must be an array of the same size."
            );
            using element_type_l = std::tuple_element_t<0, FieldL>;
            using element_type_r = std::tuple_element_t<0, FieldR>;
            if constexpr (do_member_wise_assign<element_type_l>) {
               for(size_t i = 0; i < std::tuple_size_v<FieldL>; ++i) {
                  element_type_l::visit_fields_in_tandem(lhs_field[i], rhs_field[i], _visit_members);
               }
            } else {
               lhs_field = rhs_field;
            }
         } else {
         //
         // the final/"real" code would also want to handle std::vectors of form uses, 
         // std::vectors of structs that use form data params, et cetera.
         //
            lhs_field = rhs_field;
         }
      }
   }

   template<form_data_type L, form_data_type R>
   void assign(L& lhs_form, const R& rhs_form) {
      impl::_assign_field(lhs_form, lhs_form, rhs_form);
   }

   template<form_data_type LHSForm, uses_form_data_params LHSField, uses_form_data_params RHSField>
   void assign(LHSForm& lhs_form, LHSField& lhs_subobject, const RHSField& rhs_subobject) {
      impl::_assign_field(lhs_form, lhs_subobject, rhs_subobject);
   }
}