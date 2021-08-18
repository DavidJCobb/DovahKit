#pragma once
#include "../base.h"
#include "../../wrapper.h"

class QPainterPath;

namespace dovahscript::wrappers {
   //
   // Lua class which wraps QPainterPath. Despite our use of (wrapper_metatable) here, this class 
   // doesn't use the normal (wrapper) class, instead using a custom class so that the VM core 
   // doesn't have to be altered to track the QPainterPath pointers.
   // 
   // QPainterPath is not listed in Qt's documentation as reentrant, but QPainter, which uses it, 
   // is listed as reentrant. Current working assumption is Qt just forgot to mark the former.
   //
   struct raster_draw_path : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.raster_draw_path";
      static constexpr const char*   class_name      = "raster_draw_path";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "raster_draw_path";

      // Creates a singleton for this class, and leaves it at the top of the stack.
      static void import_singleton(lua_State*);

      static QPainterPath* pull(lua_State* L, int stack_pos);
      static QPainterPath* pull_self(lua_State* L);
   };
}