#include "./build_outbound_use_info.h"
#include <array>
#include <concepts>
#include "dovah/form_type_info.h"

namespace dovah::form_data_function_tables::build_outbound_use_info {
   namespace {
      using table_type = std::array<function_type, std::tuple_size_v<decltype(form_types)>>;

      template<form_type T>
      concept entry_is_defined = requires {
         { &entry<T>::execute } -> std::same_as<function_type>;
      };

      template<form_type V>
      constexpr function_type entry_for = []() -> function_type {
         if constexpr (entry_is_defined<V>) {
            return &entry<V>::execute;
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