#include "./create_blank_instance.h"
#include "lu/passkey.h"
#include "./_helpers.inl"

namespace dovah::form_data_function_tables::create_blank_instance {
   template<form_type V>
   struct entry {
      static constexpr const bool defined = impl::index_of_form_type<V> != (size_t)-1;

      static base_managed_form_data& execute(form_stub& stub) {
         if constexpr (defined) {
            return *new impl::form_data_class_for_form_type<V>(lu::passkey<entry>{}, stub);
         } else {
            std::unreachable();
         }
      }
   };

   namespace {
      using table_type = impl::table_type<function_type>;

      template<form_type V>
      constexpr function_type entry_for = []() -> function_type {
         using entry_type = entry<V>;
         if constexpr (entry_type::defined) {
            return &entry_type::execute;
         } else {
            return nullptr;
         }
      }();

      namespace _impl {
         using index_sequence = std::make_index_sequence<std::tuple_size_v<decltype(form_types)>>;

         template<typename IS>
         struct make_table;

         template<size_t... I>
         struct make_table<std::index_sequence<I...>> {
            static constexpr table_type table = table_type{ entry_for<(form_type)I>... };
         };
      }

      static constexpr const table_type table = _impl::make_table<_impl::index_sequence>::table;
   }

   extern function_type lookup(form_type ft) {
      return table[(size_t)ft];
   }
}