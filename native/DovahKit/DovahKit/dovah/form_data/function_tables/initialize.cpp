#include "./initialize.h"
#include <array>
#include <concepts>
#include "./_helpers.inl"
#include "dovah/form_data/all_types.h"
#include "dovah/form_type_info.h"

namespace dovah::form_data::function_tables::initialize {
   namespace {
      using table_type = std::array<function_type, std::tuple_size_v<decltype(form_types)>>;

      template<form_type V>
      concept entry_is_defined = requires {
         { &entry<V>::execute } -> 
            #ifdef __INTELLISENSE__
               std::same_as<function_type>
            #else
               std::same_as<function_type&>
            #endif
         ;
      };

      template<form_type V>
      static void _cast_and_exec(base_managed_form_data& data) {
         using typed_data_type = impl::form_data_class_for_form_type<V>;
         entry<V>::execute(static_cast<typed_data_type&>(data));
      }

      template<form_type V>
      constexpr function_type entry_for = []() -> function_type {
         if constexpr (entry_is_defined<V>) {
            return &_cast_and_exec<V>;
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