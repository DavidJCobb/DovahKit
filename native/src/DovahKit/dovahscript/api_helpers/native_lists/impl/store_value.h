#pragma once
#include <type_traits>
#include "dovah/form_reference_t.h"
#include "../member_function_spec.h"
namespace dovah::loaded_forms {
   class Form;
}

namespace dovahscript::api_helpers::native_lists::impl {
   template<typename Spec>
      requires (
         // This template is valid if assignment is special-cased by the spec:
         impl::fields::store_value::valid<Spec>

         // ...or if it's special-cased in this function:
      || std::is_base_of_v<dovah::form_reference_t, typename Spec::value_stored_type>

         // ...or if it's not special-cased, but vanilla `dst = src` works:
      || std::is_assignable_v<typename Spec::value_stored_type, typename Spec::value_working_type> // default behavior
      )
      [[msvc::forceinline]] [[gnu::always_inline]]
   void exec_store_value(const typename Spec::value_working_type& src, typename Spec::value_stored_type& dst, dovah::loaded_forms::Form& dst_form) {
      if constexpr (impl::fields::store_value::valid<Spec>) {
         if constexpr (std::is_invocable_v<decltype(Spec::store_value), const typename Spec::value_working_type&, typename Spec::value_stored_type&, dovah::loaded_forms::Form&>) {
            Spec::store_value(src, dst, dst_form);
         } else {
            Spec::store_value(src, dst);
         }
      } else {
         if constexpr (std::is_base_of_v<dovah::form_reference_t, typename Spec::value_stored_type>) {
            dst.set(dst_form, src);
         } else {
            dst = src;
         }
      }
   }
}