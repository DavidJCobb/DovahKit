#include "qt_variant.h"

namespace cobb::lua {
   extern bool qt_variant_is_int(const QVariant& variant) {
      switch (variant.typeId()) {
         case QMetaType::Int:
         case QMetaType::UInt:
         case QMetaType::LongLong:
         case QMetaType::ULongLong:
            return true;
      }
      return false;
   }
   extern int type_of_qt_variant(const QVariant& variant) {
      switch (variant.typeId()) {
         case QMetaType::Bool:
            return LUA_TBOOLEAN;
         case QMetaType::Double:
         case QMetaType::Float:
            return LUA_TNUMBER;
         case QMetaType::Int:
         case QMetaType::UInt:
         case QMetaType::LongLong:
         case QMetaType::ULongLong:
            return LUA_TNUMBER;
         case QMetaType::QChar:
         case QMetaType::QString:
            return LUA_TSTRING;
         case QMetaType::QStringList:
            return LUA_TTABLE;
      }
      return LUA_TNONE;
   }

   extern int push_qt_variant(lua_State* L, const QVariant& variant) {
      if (!variant.isValid()) {
         lua_pushnil(L);
         return 1;
      }
      switch (variant.typeId()) {
         case QMetaType::Bool:
            lua_pushboolean(L, variant.toBool());
            return 1;
         case QMetaType::Double:
         case QMetaType::Float:
            lua_pushnumber(L, variant.toDouble());
            return 1;
         case QMetaType::Int:
         case QMetaType::UInt:
         case QMetaType::LongLong:
         case QMetaType::ULongLong:
            lua_pushinteger(L, variant.toInt());
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
   extern QVariant to_qt_variant(lua_State* L, int index) {
      QVariant out;
      if (lua_isinteger(L, index)) {
         out = lua_tointeger(L, index);
         return out;
      }
      switch (lua_type(L, index)) {
         case LUA_TBOOLEAN:
            out = lua_toboolean(L, index);
            break;
         case LUA_TNIL:
         case LUA_TNONE:
            break;
         case LUA_TNUMBER:
            out = lua_tonumber(L, index);
            break;
         case LUA_TSTRING:
            out = QString::fromUtf8(lua_tostring(L, index));
            break;
         case LUA_TTABLE:
            {
               lua_len(L, index);
               int len = lua_tointeger(L, -1);
               lua_pop(L, 1);
               if (!len)
                  break;
               index = lua_absindex(L, index);
               int i = len;
               do {
                  lua_geti(L, index, i);
                  bool is = lua_isstring(L, -1);
                  lua_pop(L, 1);
                  if (!is)
                     break;
               } while (--i);
               if (i)
                  break;
               //
               // All entries are strings or convertible to strings.
               //
               QStringList result;
               result.reserve(len);
               i = len;
               do {
                  lua_geti(L, index, i);
                  result.prepend(QString::fromUtf8(lua_tostring(L, -1)));
                  lua_pop(L, 1);
               } while (--i);
               out = result;
            }
            break;

      }
      return out;
   }
}
