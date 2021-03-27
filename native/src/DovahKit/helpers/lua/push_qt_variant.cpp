#include "push_qt_variant.h"

namespace cobb::lua {
   extern int push_qt_variant(lua_State* L, const QVariant& variant) {
      if (!variant.isValid()) {
         lua_pushnil(L);
         return 1;
      }
      if (variant.canConvert<int>()) {
         lua_pushinteger(L, variant.toInt());
         return 1;
      }
      switch (variant.type()) {
         case QMetaType::Bool:
            lua_pushboolean(L, variant.toBool());
            return 1;
         case QMetaType::Double:
         case QMetaType::Float:
            lua_pushnumber(L, variant.toDouble());
            return 1;
         case QMetaType::QChar:
         case QMetaType::QString:
            lua_pushstring(L, variant.toString().toUtf8());
            return 1;
         case QMetaType::QStringList:
            {
               auto sl   = variant.toStringList();
               int  size = sl.size();
               lua_createtable(L, size, 0);
               for (int i = 0; i < size; ++i) {
                  lua_pushstring(L, sl[i].toUtf8());
                  lua_rawseti(L, -2, i);
               }
               return size;
            }
      }
      lua_pushnil(L);
      return 1;
   }
}
