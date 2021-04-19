#pragma once
#include <QVariant>
#include "../../lua.h"

namespace cobb::lua {
   extern bool qt_variant_is_int(const QVariant&);
   extern int type_of_qt_variant(const QVariant&); // LUA_TNONE if unrecognized or invalid

   extern int push_qt_variant(lua_State* L, const QVariant&);
   extern QVariant to_qt_variant(lua_State* L, int index);
}
