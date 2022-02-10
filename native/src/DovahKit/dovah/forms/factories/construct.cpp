#include "construct.h"
#include "../_all.h"

namespace {
   using namespace dovah;

   using _loader_t = form_loader_function_t;
   using _construct_t = loaded_forms::Form* (*)(const loaded_forms::Form::constructor_params&);

   template<typename T> concept can_construct = requires(tes_record_reader& record, form_stub_use_info_builder& uib) {
      requires !std::is_base_of_v<loaded_forms::_IncompleteFormType, T>;
      { T::generate_use_info(record, uib) }; // can generate use info?
   };

   template<typename T> void _load(loaded_forms::Form* instance, tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      ((T*)instance)->load(record, intfc);
   }
   template<typename T> loaded_forms::Form* _construct(const loaded_forms::Form::constructor_params& c) {
      return new T(c);
   }

   struct function_table_row {
      _loader_t    load      = nullptr;
      _construct_t construct = nullptr;
   };
   using function_table_t = std::array<function_table_row, form_types.size()>;
   constexpr auto function_table = ([]() {
      function_table_t out = {};
      all_loaded_form_types::for_each([&out]<typename T>() {
         auto& row = out[T::form_type];
         if constexpr (can_construct<T>) {
            row.load      = _load<T>;
            row.construct = _construct<T>;
         }
      });
      return out;
   })();
}
namespace dovah {
   form_loader_function_t get_form_loader_function(form_type_t ft) noexcept {
      if (auto* f = function_table[ft].load)
         return f;
      return nullptr;
   }
   loaded_forms::Form* create_blank_loaded_form_by_type(form_type_t ft, const loaded_forms::Form::constructor_params& c) noexcept {
      if (auto* f = function_table[ft].construct)
         return (f)(c);
      return nullptr;
   }
}