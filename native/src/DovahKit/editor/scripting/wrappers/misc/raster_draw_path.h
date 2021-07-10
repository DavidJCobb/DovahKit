#pragma once
#include "../../wrapper.h"

class QPainterPath;

namespace editor_script::wrappers {
   //
   // Lua class which wraps QPainterPath. Despite our use of (wrapper_metatable) here, this class 
   // doesn't use the normal (wrapper) class, instead using a custom class so that the VM core 
   // doesn't have to be altered to track the QPainterPath pointers.
   // 
   // QPainterPath is not listed in Qt's documentation as reentrant, but QPainter, which uses it, 
   // is listed as reentrant. Current working assumption is Qt just forgot to mark the former.
   //
   struct raster_draw_path : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.raster_draw_path";
      static constexpr const char* class_name     = "raster_draw_path";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "raster_draw_path";

      static void setup(lua_State*);

      static QPainterPath* pull(lua_State* L, int stack_pos);
      static QPainterPath* pull_self(lua_State* L);
   };
}