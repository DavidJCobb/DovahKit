#pragma once
#include <cassert>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include "helpers/tuples/all_types_match_functor.h"
#include "helpers/tuples/for_each_nttp_value.h"
#include "helpers/tuples/for_each_value.h"
#include "lua.h"
#include "./is_property_tuple_type.h"
#include "./property_definition.h"
namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_reference_t;
}

namespace dovahscript::api_helpers::subobject_property_helpers {
   template<typename Dst, bool NilMeansUnchanged, const auto& PropertiesTuple>
      requires (
         is_property_tuple_type<std::decay_t<decltype(PropertiesTuple)>>
         #ifndef __INTELLISENSE__ // 04/20/2026: IntelliSense hates `all_types_match_functor`
         &&
         cobb::tuples::all_types_match_functor<
            std::decay_t<decltype(PropertiesTuple)>,
            []<typename Value>() {
               return std::is_same_v<Dst, typename Value::subobject_type>;
            }
         >
         #endif
      )
   void apply_table(
      lua_State* L,
      int table_pos,
      dovah::loaded_forms::Form& dst_form,
      Dst& dst_data
   ) {
      // 04/20/2026:
      // We should be able to reference this template parameter in if-constexpr directly, 
      // but at least in Debug, MSVC is compiling incorrectly and causing it to always 
      // act as `false` in if-constexpr.
      constexpr const bool nil_means_unchanged = NilMeansUnchanged;

      assert(lua_istable(L, table_pos) || lua_isuserdata(L, table_pos));

      cobb::tuples::for_each_nttp_value<
         PropertiesTuple,
         []<const auto& PropertyDefinition>(
            lua_State* L,
            int table_pos,
            dovah::loaded_forms::Form& dst_form,
            Dst& dst_data
         ) [[msvc::forceinline]] [[gnu::always_inline]] {
            using stored_type = typename std::decay_t<decltype(PropertyDefinition)>::stored_type;
            using value_type  = typename std::decay_t<decltype(PropertyDefinition)>::value_type;

            auto& dst_field = PropertyDefinition.access(dst_data);

            lua_getfield(L, table_pos, PropertyDefinition.name.data());
            if (lua_isnoneornil(L, -1)) {
               lua_pop(L, 1);
               if constexpr (!nil_means_unchanged && !PropertyDefinition.treat_nil_as_unchanged) {
                  if constexpr (std::is_base_of_v<dovah::form_reference_t, stored_type>) {
                     dst_field.set(dst_form, nullptr);
                  } else if constexpr (std::is_base_of_v<dovah::localized_string, stored_type>) {
                     dst_field.reset();
                  } else if constexpr (PropertyDefinition.default_value.has_value()) {
                     dst_field = PropertyDefinition.default_value.value();
                  } else {
                     dst_field = {};
                  }
               }
            } else {
               auto v = PropertyDefinition.pull(L, -1);
               lua_pop(L, 1);
               if constexpr (std::is_base_of_v<dovah::form_reference_t, stored_type>) {
                  dst_field.set(dst_form, v);
               } else if constexpr (std::is_base_of_v<dovah::localized_string, stored_type> && std::is_same_v<value_type, std::string_view>) {
                  dst_field = std::string(v); // HACK because there's no assignment operator for string-view and I don't wanna recompile the whole program to add one
               } else {
                  dst_field = v;
               }
            }
         }
      >(L, table_pos, dst_form, dst_data);
   }
}