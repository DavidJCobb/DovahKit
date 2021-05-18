#pragma once

struct lua_State;

namespace editor_script {
   extern void build_all_resource_wrapper_singletons(lua_State*);
   extern void build_all_ui_wrapper_singletons(lua_State*); // "ui" table should be at the top of the stack when this is called
}