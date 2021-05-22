#include "raster.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/lua_managed_resources.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"
#include "../../ui/util/color.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::resource::raster;

   namespace _methods {
      luastackchange_t get_pixel(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         //
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,  2, "x-coordinate (integer) expected");
         luaL_argcheck(L, x != 0, 2, "x-coordinate cannot be zero");
         luaL_argcheck(L, x >= 0, 2, "x-coordinate cannot be negative");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum,  3, "y-coordinate (integer) expected");
         luaL_argcheck(L, y != 0, 3, "y-coordinate cannot be zero");
         luaL_argcheck(L, y >= 0, 3, "y-coordinate cannot be negative");
         --x;
         --y;
         //
         auto image = self.managed_resource->get_raster_script_side();
         util::ui::push_color(L, image.pixel(x, y));
         return 1;
      }
      luastackchange_t set_pixel(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         //
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,  2, "x-coordinate (integer) expected");
         luaL_argcheck(L, x != 0, 2, "x-coordinate cannot be zero");
         luaL_argcheck(L, x >= 0, 2, "x-coordinate cannot be negative");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum,  3, "y-coordinate (integer) expected");
         luaL_argcheck(L, y != 0, 3, "y-coordinate cannot be zero");
         luaL_argcheck(L, y >= 0, 3, "y-coordinate cannot be negative");
         --x;
         --y;
         QColor color = util::ui::pull_color(L, 4);
         //
         self.managed_resource->modify_raster_script_side([x, y, color](QImage& image) {
            assert(image.format() == QImage::Format::Format_ARGB32);
            auto* bytes = (QRgb*)image.scanLine(y);
            bytes[x] = color.rgba();
         });
         //
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         auto image = self.managed_resource->get_raster_script_side();
         if (image.isNull())
            return 0;
         lua_pushinteger(L, image.height());
         return 1;
      }
      luastackchange_t width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         auto image = self.managed_resource->get_raster_script_side();
         if (image.isNull())
            return 0;
         lua_pushinteger(L, image.width());
         return 1;
      }
   }
   namespace _setters {
      /*//
      luastackchange_t auto_default(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QPushButton::setAutoDefault, value);
         return 0;
      }
      luastackchange_t flat(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QPushButton::setFlat, value);
         return 0;
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QPushButton::setText, value);
         return 0;
      }
      //*/
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         int    width      = -1;
         int    height     = -1;
         QColor background = Qt::GlobalColor::transparent;
         lua_settop(L, 1);
         {
            int isnum;
            int type = lua_type(L, 1);
            if (type != LUA_TTABLE && type != LUA_TUSERDATA)
               luaL_error(L, "expected options table or userdata as argument; got %s", lua_typename(L, type));
            lua_getfield(L, 1, "width");
            width = lua_tointegerx(L, 2, &isnum);
            luaL_argcheck(L, isnum, 2, "expected integer width");
            lua_pop(L, 1);
            lua_getfield(L, 1, "height");
            height = lua_tointegerx(L, 2, &isnum);
            luaL_argcheck(L, isnum, 2, "expected integer height");
            lua_pop(L, 1);
            //
            luaL_argcheck(L, width  >= 0, 1, "width cannot be negative");
            luaL_argcheck(L, width  != 0, 1, "width cannot be zero");
            luaL_argcheck(L, height >= 0, 1, "height cannot be negative");
            luaL_argcheck(L, height != 0, 1, "height cannot be zero");
            //
            lua_getfield(L, 1, "background_color");
            if (!lua_isnoneornil(L, 2)) {
               background = util::ui::pull_color(L, 2);
            }
            lua_pop(L, 1);
         }
         //
         LuaManagedResource* resource = nullptr;
         {
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [width, height, background, &resource]() {
               auto base = QImage(width, height, QImage::Format::Format_ARGB32);
               base.fill(background);
               resource = DovahKitScriptVMResourceInterface::get().create_resource(base);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
            assert(resource);
         }
         return cls::wrap_and_push(L, *resource);
      }
      luastackchange_t is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace editor_script::wrappers::resource {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "get_pixel", &_methods::get_pixel },
      { "set_pixel", &_methods::set_pixel },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "height", &_getters::height },
      { "width",  &_getters::width },
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

   /*static*/ int cls::wrap_and_push(lua_State* L, LuaManagedResource& resource) {
      wrapper out;
      out.type = wrapper_type::lua_managed_resource;
      out.managed_resource = &resource;
      return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::metatable_key);
   }
}