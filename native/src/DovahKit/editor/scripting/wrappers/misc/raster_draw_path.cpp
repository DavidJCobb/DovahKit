#include "raster_draw_path.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"

#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"

#include "../../../../helpers/lua/error.h"
#include "../../../../helpers/lua/istablelike.h"
#include "../../../../helpers/lua/qt_point.h"
#include "../../../../helpers/lua/set_top_on_exit.h"

#include <QPainterPath>

namespace {
   using namespace editor_script;
   using cls = wrappers::raster_draw_path;

   struct _wrapped_path {
      bool          dead = false;
      QPainterPath* path = nullptr;

      void teardown() {
         if (this->dead)
            return;
         this->dead = true;
         delete this->path;
         this->path = nullptr;
      }

      static _wrapped_path* create(lua_State* L) {
         auto* memory   = lua_newuserdata(L, sizeof(_wrapped_path));
         auto* instance = new (memory) _wrapped_path;
         //
         lua_getfield(L, LUA_REGISTRYINDEX, cls::metatable_key); // push 1
         if (lua_isnoneornil(L, -1)) {
            assert(false && "The wrapper-class wasn't set up properly; its metatable is undefined.");
            cobb::lua::error(L, "internal program error: the metatable for raster_draw_path wasn't set up properly, so instances cannot be created");
            return 0;
         }
         lua_setmetatable(L, -2); // pop 1
         //
         return instance;
      }
      
      static int __close(lua_State* L) {
         auto* userdata = (_wrapped_path*) lua_touserdata(L, 1);
         userdata->teardown();
         return 0;
      }
      static int __gc(lua_State* L) {
         auto* userdata = (_wrapped_path*) lua_touserdata(L, 1);
         userdata->teardown();
         userdata->~_wrapped_path();
         lua_pushnil(L);
         lua_setmetatable(L, 1); // Lua can't guarantee that __gc will only be called once, so make sure there *is* no __gc to call a second time
         return 0;
      }
   };

   namespace _methods {
      luastackchange_t line_to(lua_State* L) {
         auto* path = cls::pull_self(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number (x-coordinate) expected");
         luaL_argcheck(L, lua_isnumber(L, 3), 3, "number (y-coordinate) expected");
         qreal x = lua_tonumber(L, 2);
         qreal y = lua_tonumber(L, 3);
         path->lineTo(x, y);
         return 0;
      }
      luastackchange_t move_to(lua_State* L) {
         auto* path = cls::pull_self(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number (x-coordinate) expected");
         luaL_argcheck(L, lua_isnumber(L, 3), 3, "number (y-coordinate) expected");
         qreal x = lua_tonumber(L, 2);
         qreal y = lua_tonumber(L, 3);
         path->moveTo(x, y);
         return 0;
      }
   }
   namespace _getters {
   }
   namespace _setters {
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            luaL_error(L, "the ui.canvas.new function should not be called with a colon or passed any arguments");
         //
         _wrapped_path* instance = _wrapped_path::create(L); // pushes a userdata onto the Lua stack
         {
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [instance]() {
               instance->path = new QPainterPath;
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         return 1;
      }
      luastackchange_t is(lua_State* L) {
         auto* wrapper = editor_script::cast_to_class(L, 1, cls::metatable_key);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "__close", &_wrapped_path::__close }, // This userdata doesn't derive from (wrapper), so it needs its own GC code
      { "__gc",    &_wrapped_path::__gc },    //
      { "line_to", &_methods::line_to },
      { "move_to", &_methods::move_to },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setglobal(L, cls::global_name);
   }

   /*static*/ QPainterPath* cls::pull(lua_State* L, int stack_pos) {
      auto* wrapper = (_wrapped_path*) editor_script::cast_to_class(L, stack_pos, cls::metatable_key);
      if (!wrapper)
         return nullptr;
      return wrapper->path;
   }
   /*static*/ QPainterPath* cls::pull_self(lua_State* L) {
      auto* wrapper = (_wrapped_path*)editor_script::cast_to_class(L, 1, cls::metatable_key);
      if (!wrapper)
         cobb::lua::error(L, "raster_draw_path method called on bad self");
      return wrapper->path;
   }
}