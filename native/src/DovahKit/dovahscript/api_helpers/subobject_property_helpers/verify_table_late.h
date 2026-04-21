#pragma once
#include <cassert>
#include <type_traits>
#include "helpers/tuples/for_each_nttp_value.h"
#include "lua.h"
#include "./is_property_tuple_type.h"
#include "./property_definition.h"
#include "./utils/property_list_has_late_checks.h"
namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_reference_t;
}

namespace dovahscript::api_helpers::subobject_property_helpers {
   template<typename Dst, bool NilMeansUnchanged, const auto& PropertiesTuple>
      requires is_property_tuple_type<std::decay_t<decltype(PropertiesTuple)>>
   void verify_table_late(
      lua_State* L,
      int table_pos,
      dovah::loaded_forms::Form& dst_form,
      Dst* dst_data
   ) {
      assert(lua_istable(L, table_pos) || lua_isuserdata(L, table_pos));

      if constexpr (utils::property_list_has_late_checks<PropertiesTuple>) {
         cobb::tuples::for_each_nttp_value<
            PropertiesTuple,
            []<const auto& PropertyDefinition>(
               lua_State* L,
               int table_pos,
               dovah::loaded_forms::Form& dst_form,
               Dst* dst_data
            ) {
               if constexpr (PropertyDefinition.late_check) {
                  lua_getfield(L, table_pos, PropertyDefinition.name.data());
                  if (lua_isnoneornil(L, -1)) {
                     lua_pop(L, 1);
                  } else {
                     auto v = PropertyDefinition.pull(L, -1);
                     lua_pop(L, 1);
                     PropertyDefinition.late_check(L, dst_form, dst_data, v);
                  }
               }
            }
         >(L, table_pos, dst_form, dst_data);
      }
   }
}