#include "construct.h"
#include "../_all.h"
#include "./use_info.h"

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
         auto& row = out[(size_t)T::form_type];
         if constexpr (can_construct<T>) {
            row.load      = _load<T>;
            row.construct = _construct<T>;
         }
      });
      return out;
   })();
}
namespace dovah {
   form_loader_function_t get_form_loader_function(form_type ft) noexcept {
      if (get_outbound_uses_builder_by_type(ft) == nullptr)
         //
         // If we haven't generated use info for a form type, then it's not safe to load 
         // its full data, as all use info machinery built into forms assumes that use 
         // info will have been generated during the file load step.
         //
         return nullptr;

      if (auto* f = function_table[(size_t)ft].load)
         return f;
      return nullptr;
   }
   loaded_forms::Form* create_blank_loaded_form_by_type(form_type ft, const loaded_forms::Form::constructor_params& c) noexcept {
      if (auto* f = function_table[(size_t)ft].construct)
         return (f)(c);
      return nullptr;
   }

   extern bool can_construct_form_data(form_type ft) noexcept {
      return function_table[(size_t)ft].construct != nullptr;
   }
   extern bool can_load_form_data(form_type ft) noexcept {
      if (get_outbound_uses_builder_by_type(ft) == nullptr)
         //
         // If we haven't generated use info for a form type, then it's not safe to load 
         // its full data, as all use info machinery built into forms assumes that use 
         // info will have been generated during the file load step.
         //
         return false;

      return function_table[(size_t)ft].load != nullptr;
   }
}