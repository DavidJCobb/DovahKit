#pragma once
#include <QVariant>

struct lua_State;

namespace dovahscript::api_helpers {
   extern QVariant pull_variant(lua_State* L, int stack_pos); // requires the script thread
}