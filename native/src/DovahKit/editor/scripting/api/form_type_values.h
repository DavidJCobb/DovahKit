#pragma once
#include "../../../dovah/core.h"

struct lua_State;

namespace editor_script {
   extern void expose_form_types_to_lua(lua_State* L);
   extern dovah::form_type_t get_form_type_from_stack(lua_State* L, int stack_pos, bool& valid);
}