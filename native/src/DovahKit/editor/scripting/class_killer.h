#pragma once
#include "util.h"

namespace editor_script {
   constexpr char* dead_class_metatable_storage = "__dead_classes"; // registry.__dead_classes[class_name] == dummy metatable

   luastackchange_t zombify_userdata(lua_State*); // call via lua_call, not directly
}