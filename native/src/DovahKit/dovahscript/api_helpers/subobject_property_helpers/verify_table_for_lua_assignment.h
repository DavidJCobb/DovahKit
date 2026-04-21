#pragma once
#include <type_traits>
#include "helpers/lua/error.h"
#include "helpers/tuples/for_each_nttp_value.h"
#include "lua.h"
#include "./utils/property_list_has_late_checks.h"
#include "./verify_table.h"
#include "./verify_table_late.h"
namespace dovah::loaded_forms {
   class Form;
}

namespace dovahscript::api_helpers::subobject_property_helpers {
   // Collections that contain a given sub-object type can use this to validate 
   // assignments to collection indices i.e. `collection[n] = {}`.
   template<typename Subobject, const auto& PropertyList>
   void verify_table_for_lua_assignment(lua_State* L, int pos, dovah::loaded_forms::Form& dst_form, Subobject& dst_subobject) {
      {
         auto message = subobject_property_helpers::verify_table<Subobject, false, PropertyList>(L, pos);
         if (!message.empty())
            cobb::lua::error(L, "assignment source is not valid: %s", message.data());
      }
      if constexpr (subobject_property_helpers::utils::property_list_has_late_checks<PropertyList>) {
         subobject_property_helpers::verify_table_late<Subobject, false, PropertyList>(L, pos, dst_form, &dst_subobject);
      }
   }

   // Collections that contain a given sub-object type can use this to validate 
   // insertions. This verifies that a wholly new sub-object of the given type 
   // can be constructed inside a given form using a given table.
   template<typename Subobject, const auto& PropertyList>
   void verify_table_for_lua_insertion(lua_State* L, int pos, dovah::loaded_forms::Form& dst_form) {
      {
         auto message = subobject_property_helpers::verify_table<Subobject, false, PropertyList>(L, pos);
         if (!message.empty())
            cobb::lua::error(L, "cannot store this data: %s", message.data());
      }
      if constexpr (subobject_property_helpers::utils::property_list_has_late_checks<PropertyList>) {
         subobject_property_helpers::verify_table_late<Subobject, false, PropertyList>(L, pos, dst_form, nullptr);
      }
   }
}