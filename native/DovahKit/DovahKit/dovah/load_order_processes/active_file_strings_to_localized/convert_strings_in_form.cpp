#include "./convert_strings_in_form.h"
#include <array>
#include <utility>
#include "lu/concepts/instantiation_of/std_array.h"
#include "dovah/form_data/all_types.h"
#include "dovah/form_data_function_tables/_helpers.inl"
#include "dovah/form_data/helpers/is_or_contains_managed_string_use.h"
#include "dovah/form_data/helpers/is_visitable.h"
#include "dovah/form_stub.h"
#include "dovah/form_type.h"
#include "./process_pimpl.h"

namespace dovah::load_order_processes::active_file_strings_to_localized {
   namespace {
      struct convert_state {
         form_stub&     containing_form;
         process_pimpl& pimpl;
      };
   }

   #pragma region _convert_all_strings_in
      template<localization::category C>
      static void _convert_all_strings_in(const convert_state& state, managed_localized_string_use<C>& use) {
         state.pimpl.convert_localized_string(
            state.containing_form,
            C,
            use
         );
      }

      template<lu::concepts::instantiation_of_std_array T>
      static void _convert_all_strings_in(const convert_state& state, T& data) {
         for (auto& item : data)
            _convert_all_strings_in(state, item);
      }

      template<form_data_helpers::is_visitable T>
      static void _convert_all_strings_in(const convert_state& state, T& data) {
         if constexpr (form_data_helpers::is_or_contains_managed_string_use<T>) {
            T::visit_fields(data, [&state](auto& field) {
               using field_type = std::decay_t<decltype(field)>;
               if constexpr (form_data_helpers::is_or_contains_managed_string_use<field_type>) {
                  _convert_all_strings_in(state, field);
               }
            });
         }
      }
   #pragma endregion

   namespace function_table_impl {
      using function_type = void(*)(const convert_state&, base_managed_form_data&);
      using table_type    = std::array<function_type, std::tuple_size_v<decltype(form_types)>>;
      
      template<typename T>
      static void _cast_and_exec(const convert_state& state, base_managed_form_data& form_data) {
         _convert_all_strings_in(state, static_cast<T&>(form_data));
      }
      
      template<form_type V>
      struct _get_entry {
         static constexpr const function_type value = nullptr;
      };
      //
      template<form_type V>
         requires (form_data_function_tables::impl::index_of_form_type<V> != (size_t)-1)
      struct _get_entry<V> {
         using loaded_form_data = form_data_function_tables::impl::form_data_class_for_form_type<V>;
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

   extern void convert_strings_in_form(
      process_pimpl& pimpl,
      form_stub& stub,
      base_managed_form_data& data
   ) {
      auto handler = function_table_impl::table[(size_t)stub.form_type];
      if (!handler)
         return;

      const convert_state state = {
         .containing_form = stub,
         .pimpl           = pimpl,
      };
      handler(state, data);
   }
}