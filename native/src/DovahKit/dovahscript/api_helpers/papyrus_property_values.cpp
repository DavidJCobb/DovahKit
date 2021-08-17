#include "papyrus_property_values.h"
#include "../wrapper.h"
#include "../wrappers/form/form.h"
#include "../wrappers/form/papyrus/property.h"
#include "../wrappers/form/quest/alias.h"

namespace dovahscript::api_helpers::papyrus {
   extern bool property_scalar_value_typecheck(lua_State* L, int stack_pos, papyrus_property_type pt) {
      pt = dovah::loaded_forms::components::papyrus::scalar_property_type_for(pt);
      switch (pt) {
         case papyrus_property_type::boolean:
            return lua_isboolean(L, stack_pos);
         case papyrus_property_type::float32:
            return lua_isnumber(L, stack_pos);
         case papyrus_property_type::integer:
            {
               int isnum;
               lua_tointegerx(L, stack_pos, &isnum);
               return isnum != 0;
            }
            break;
         case papyrus_property_type::object:
            {
               if (lua_isnoneornil(L, stack_pos))
                  return true;
               stack_pos = lua_absindex(L, stack_pos);
               if (auto* form = wrapper_from_stack<wrappers::form>(L, stack_pos))
                  return true;
               if (auto* alias = wrapper_from_stack<wrappers::quest_alias>(L, stack_pos))
                  return true;
               return false;
            }
            break;
         case papyrus_property_type::string:
            return lua_isstring(L, stack_pos);
      }
      return false;
   }

   extern void set_property_value(lua_State* L, int stack_pos, wrapper& wrapper, wrapped_property& prop, size_t index) {
      auto& lf = *wrapper.form;
      auto  st = prop.scalar_type();
      auto& v  = prop.values[index];
      prop.values[index].clear(lf);
      switch (st) {
         case papyrus_property_type::boolean:
            v.boolean = lua_toboolean(L, stack_pos);
            break;
         case papyrus_property_type::float32:
            v.float32 = lua_tonumber(L, stack_pos);
            break;
         case papyrus_property_type::integer:
            {
               int isnum;
               int i = lua_tointegerx(L, stack_pos, &isnum);
               assert(isnum); // we should already have checked this, above
               v.integer = i;
            }
            break;
         case papyrus_property_type::object:
            if (lua_isnoneornil(L, stack_pos))
               break;
            if (auto* form = wrapper_from_stack<wrappers::form>(L, stack_pos)) {
               v.object.form.set(lf, form->stub);
               break;
            }
            if (auto* alias_wrapper = wrapper_from_stack<wrappers::quest_alias>(L, stack_pos)) {
               if (auto* alias = wrappers::quest_alias::unwrap(*alias_wrapper)) {
                  v.object.form.set(lf, alias_wrapper->stub);
                  v.object.aliasID = alias->id;
               }
               break;
            }
            break;
         case papyrus_property_type::string:
            v.string = lua_tostring(L, stack_pos);
            break;
      }
   }
}