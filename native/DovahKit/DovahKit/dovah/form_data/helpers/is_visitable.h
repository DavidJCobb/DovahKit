#pragma once

namespace dovah::form_data_helpers {
   namespace _impl {
      template<typename FormData, typename Field>
      void dummy_non_const_visitor(FormData&, Field&) {
      }
      template<typename FormData, typename Field>
      void dummy_const_visitor(const FormData&, const Field&) {
      }
   }

   template<typename T>
   concept is_visitable = requires(T& f, const T& cf) {
      { T::visit_members(f,  &_impl::dummy_non_const_visitor) };
      { T::visit_members(cf, &_impl::dummy_const_visitor) };
   };
}