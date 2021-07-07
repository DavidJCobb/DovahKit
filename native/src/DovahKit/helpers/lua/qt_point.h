#pragma once
#include <QPoint>
#include "../../lua.h"

namespace cobb::lua {
   extern QPoint  pull_qpoint(lua_State* L, int index, const char* error_prefix);
   extern QPointF pull_qpointf(lua_State* L, int index, const char* error_prefix);
}
