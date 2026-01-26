#include "./extra_data_type_has_post_load_validation.h"
#include <array>
#include "../types/all.h"
#include "../types/class_array.h"

namespace dovah::loaded_forms::components::extra_data_utils {
   namespace {
      using extra_data    = extra_data_types::extra_data;
      using typecode_type = extra_data::typecode_type;

      template<typename T>
      constexpr const bool has_post_load_validation = !std::is_same_v<
         decltype(&extra_data::post_load_validation),
         decltype(&T::post_load_validation)
      >;

      constexpr const auto presence = []() {
         constexpr const size_t bytecount = all_extra_data_types::count / 8 + ((all_extra_data_types::count % 8) ? 1 : 0);
         std::array<uint8_t, bytecount> bytes = {};
         size_t i = 0;
         all_extra_data_types::for_each([&bytes, &i]<typename T>() {
            size_t byte = i / 8;
            size_t bit  = i % 8;
            bytes[byte] |= (has_post_load_validation<T> ? 1 : 0) << bit;
            ++i;
         });
         return bytes;
      }();

      constexpr const bool check_presence(typecode_type type) {
         size_t byte = (size_t)type / 8;
         size_t bit  = (size_t)type % 8;
         return ((presence[byte] >> bit) & 1) != 0;
      }

      // Quick check against a type that we know overloads the function.
      static_assert(check_presence(all_extra_data_types::index_of_type<extra_data_types::linked_ref>));
   }

   extern bool extra_data_type_has_post_load_validation(typecode_type type) {
      return check_presence(type);
   }

   extern bool extra_data_has_post_load_validation(const extra_data_types::extra_data& e) {
      return extra_data_type_has_post_load_validation(e.typecode);
   }
}