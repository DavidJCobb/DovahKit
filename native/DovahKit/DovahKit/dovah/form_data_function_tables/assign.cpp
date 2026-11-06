#include "./assign.h"
#include <array>
#include "./_helpers.inl"
#include "../form_data/all_types.h"
#include "../form_data/operations/clear_outbound_uses.h"
#include "../form_type_info.h"

namespace {
   template<typename T>
   static void _clear_outbound_uses_from_form_data(dovah::base_managed_form_data& form_data) {
      dovah::form_data_operations::clear_outbound_uses(form_data.stub, static_cast<T&>(form_data));
   }
}

namespace dovah::form_data_function_tables::assign {
   namespace managed_to_managed {
      namespace {
         using table_type = std::array<function_type, std::tuple_size_v<decltype(form_types)>>;

         template<typename T>
         static void _cast_and_exec(dovah::base_managed_form_data& dst_data, const dovah::base_managed_form_data& src_data) {
            dovah::form_data_operations::assign(dst_data.stub, static_cast<T&>(dst_data), static_cast<const T&>(src_data));
         }

         template<form_type V>
         constexpr function_type entry_for = []() -> function_type {
            static constexpr const bool form_data_type_exists = impl::index_of_form_type<V> != (size_t)-1;
            if constexpr (form_data_type_exists) {
               using loaded_form_data = impl::form_data_class_for_form_type<V>;
               return &_cast_and_exec<loaded_form_data>;
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
}