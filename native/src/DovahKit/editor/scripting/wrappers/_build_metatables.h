#pragma once

struct lua_State;

namespace editor_script {
   extern void build_all_wrapper_metatables(lua_State*);
}