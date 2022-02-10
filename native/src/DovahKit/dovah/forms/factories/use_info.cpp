#include "use_info.h"
#include "../_all.h"

namespace {
   using namespace dovah;

   template<typename T> concept can_generate_use_info = requires(tes_record_reader& record, form_stub_use_info_builder& uib) {
      { T::generate_use_info(record, uib) };
   };

   using function_table_t = std::array<outbound_uses_builder_t, form_types.size()>;
   constexpr auto function_table = ([]() {
      function_table_t out = {};
      all_loaded_form_types::for_each([&out]<typename T>() {
         if constexpr (can_generate_use_info<T>) {
            out[T::form_type] = T::generate_use_info;
         } else {
            out[T::form_type] = nullptr;
         }
      });
      return out;
   })();
}
namespace dovah {
   outbound_uses_builder_t get_outbound_uses_builder_by_type(form_type_t ft) noexcept {
      return function_table[ft];
   }
}