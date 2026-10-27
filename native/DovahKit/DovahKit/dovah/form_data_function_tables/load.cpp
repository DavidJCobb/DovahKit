#include "./load.h"
#include <array>
#include "./_helpers.inl"

namespace dovah::form_data_function_tables::load {
   namespace {
      using table_type = impl::table_type<function_type>;

      template<form_type V>
      concept entry_is_defined = requires(impl::form_data_class_for_form_type<V>& data, tes::record_view& record, load_order_interfaces::form_load& intfc) {
         { serialize_form_data::load(data, record, intfc) };
      };

      template<form_type V>
      static void _adapted_load(base_managed_form_data& data, tes::record_view& record, load_order_interfaces::form_load& intfc) {
         using typed_data_type = impl::form_data_class_for_form_type<V>;
         serialize_form_data::load(static_cast<typed_data_type&>(data), record, intfc);
      }

      template<form_type V>
      constexpr function_type entry_for = []() -> function_type {
         if constexpr (entry_is_defined<V>) {
            return &_adapted_load<V>;
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