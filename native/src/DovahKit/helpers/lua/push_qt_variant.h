#pragma once
#include <QVariant>
#include "../../../Lua/lua.hpp"

namespace cobb::lua {
   extern void push_qt_variant(lua_State* L, const QVariant&);
}
