#pragma once

struct lua_State;

namespace editor_script::classes {
   extern void build_all_userdata_class_metatables(lua_State*);
}