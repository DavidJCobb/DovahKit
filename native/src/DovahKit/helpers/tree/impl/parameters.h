#pragma once
#include <bitset>
#include "../../class_array.h"
#include "../node_data_attribute.h"
#include "./strip_node_data_options.h"
#include "./typecode.h"

namespace cobb::impl::_node {
   template<typename... Parameters>
   struct parameters {
      parameters() = delete;
      ~parameters() = delete;

      static constexpr const size_t count = sizeof...(Parameters);
      static_assert(count > 0);

      using all_types = cobb::class_array<typename strip_node_data_options<Parameters>::type...>;
      using typecode_t = typecode<all_types::count>;

      template<typename T, node_data_attribute A>
      static constexpr const bool data_has_attribute = []() {
         if constexpr (is_node_data_with_attributes<T>) {
            for (auto attr : T::all_specified_attributes)
               if (attr == A)
                  return true;
         }
         return false;
      }();

      // Intentionally empty no-op type.
      struct dummy {};

      #pragma region Flag attributes
      ///
      /// Certain attributes are flags that alter  the basic run-time behavior of a node; 
      /// the "leaf" flag, for example, disallows a node from containing child nodes. The 
      /// naive approach to these flags would be to store an array  (or bitset) that maps 
      /// all data type indices to a bool indicating  the presence or absence of the flag 
      /// for that type. However, we can do slightly better.
      /// 
      /// If all of the data types have, or lack,  the flag, then there's no need for the 
      /// bitset.  The machinery here can be used to conditionally generate a bitset only 
      /// when it's needed.
      /// 
      /// To test the value of a flag F for a given type T whose typecode is C,  you want 
      /// to call:
      /// 
      ///   node_parameters<...>::flags_per_data_type<F>::has_flag(C)
      ///

      template<node_data_attribute Attr>
      struct attribute_flag_info {
         attribute_flag_info() = delete;
         ~attribute_flag_info() = delete;

         static constexpr const size_t count   = ((size_t)data_has_attribute<Parameters, Attr> +...);
         static constexpr const bool   varies  = count != all_types::count && count != 0;
         static constexpr const bool   all_set = count == all_types::count;
      };

      template<node_data_attribute Attr>
      struct flags_per_data_type;

      /// Specialization used if all data types have, or lack, the flag.
      template<node_data_attribute Attr> requires (!attribute_flag_info<Attr>::varies)
      struct flags_per_data_type<Attr> {
         flags_per_data_type() = delete;
         ~flags_per_data_type() = delete;

         static constexpr const bool has_flag(typecode_t) noexcept {
            return attribute_flag_info<Attr>::all_set;
         }
      };

      /// Specialization used when the data types vary.
      template<node_data_attribute Attr> requires attribute_flag_info<Attr>::varies
      struct flags_per_data_type<Attr> {
         flags_per_data_type() = delete;
         ~flags_per_data_type() = delete;

         static constexpr const auto mask = []() {
            std::bitset<all_types::count> out = {};
            {
               size_t i = 0;
               (out.set(i++, data_has_attribute<Parameters, Attr>), ...);
            }
            return out;
         }();

         static constexpr const bool has_flag(typecode_t type) noexcept {
            return mask.test(type);
         }
      };
      #pragma endregion
   };
}
