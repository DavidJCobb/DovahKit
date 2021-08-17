#pragma once
#include "../../dovah/forms/components/papyrus.h"
#include "../../lua.h"

namespace dovahscript {
   class wrapper;
}

namespace dovahscript::api_helpers::papyrus {
   using wrapped_property      = dovah::loaded_forms::components::papyrus::script_data::property;
   using papyrus_property_type = dovah::loaded_forms::components::papyrus::property_type;

   extern bool property_scalar_value_typecheck(lua_State* L, int stack_pos, papyrus_property_type pt);

   extern void set_property_value(lua_State* L, int stack_pos, wrapper& wrapper, wrapped_property& prop, size_t index);
}