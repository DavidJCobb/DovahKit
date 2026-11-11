#include "./assign.h"
#include <array>
#include "dovah/form_type_info.h"
#include "../all_types.h"
#include "../operations/assign.h"
#include "./_helpers.inl"

namespace {
   template<typename T>
   static void _clear_outbound_uses_from_form_data(dovah::base_managed_form_data& form_data) {
      dovah::form_data::operations::clear_outbound_uses(form_data.stub, static_cast<T&>(form_data));
   }
}

namespace dovah::form_data::function_tables::assign {
   namespace managed_to_managed {
      namespace {
         using table_type = std::array<function_type, std::tuple_size_v<decltype(form_types)>>;

         template<typename T>
         static void _cast_and_exec(dovah::base_managed_form_data& dst_data, const dovah::base_managed_form_data& src_data) {
            dovah::form_data::operations::assign(dst_data.stub, static_cast<T&>(dst_data), static_cast<const T&>(src_data));
         }

         template<form_type V>
         struct _get_entry {
            static constexpr const function_type value = nullptr;
         };
         //
         template<form_type V>
            requires (impl::index_of_form_type<V> != (size_t)-1)
         struct _get_entry<V> {
            using loaded_form_data = impl::form_data_class_for_form_type<V>;
            static constexpr const function_type value = &_cast_and_exec<loaded_form_data>;
         };

         namespace _impl {
            using index_sequence = std::make_index_sequence<std::tuple_size_v<decltype(form_types)>>;

            template<typename IS>
            struct make_table;

            template<size_t... I>
            struct make_table<std::index_sequence<I...>> {
               static constexpr table_type table = table_type{ _get_entry<(form_type)I>::value... };
            };
         }

         static constexpr const table_type table = _impl::make_table<_impl::index_sequence>::table;
      }

      extern function_type lookup(form_type ft) {
         return table[(size_t)ft];
      }
   }
}