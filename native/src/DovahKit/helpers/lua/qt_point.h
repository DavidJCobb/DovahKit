#pragma once
#include <QPoint>
#include "../../lua.h"

namespace cobb::lua {
   extern QPoint  pull_qpoint(lua_State* L, int index, const char* error_prefix);
   extern QPointF pull_qpointf(lua_State* L, int index, const char* error_prefix);

   // Return codes:
   //  0 = Success
   // -1 = Not a table
   // -2 = No X
   // -3 = No Y
   extern int pull_qpoint_float(lua_State* L, int index, QPointF& out);
}
