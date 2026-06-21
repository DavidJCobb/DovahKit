#include "dovah.h"
#include <array>
#include <cassert>
#include <QApplication>
#include <QRegularExpression>
#include "helpers/lua/dump.h"
#include "helpers/lua/error.h"
#include "helpers/lua/setfuncs.h"
#include "../core/subsystems/permissions.h"
#include "../core/subsystems/resources.h"
#include "../core/subsystems/resources/DovahscriptResource.h"
#include "../push_native_object.h"
#include "../send_script_task.h"

#include "form_types.h"

#include "../core/classes.h"
#include "../tasks/s2m/create_form.h"
#include "../tasks/s2m/lambda.h"
#include "../tasks/s2m/log_message.h"
#include "../wrapper.h"
#include "../wrappers/form/form.h"
#include "editor/core.h"

#include "dovah/data/ini_settings.h"

#include "../lua_classes/benchmark.h"

#include "editor/subsystems/game_inis.h"
#include "../wrappers/ini/setting.h"

#include "../api_helpers/load_resource.h"

namespace {
   static constexpr const char* string_format_registry_key = "dovahscript.internal.dovah.string_format_copy";
}

namespace {
   using namespace dovahscript;
   
   namespace _definitions {
      int count_forms_of_type(lua_State* L) {
         auto& editor = DovahKitCore::get();
         if (!editor.has_data()) {
            lua_pushinteger(L, 0);
            return 1;
         }
         //
         bool  valid = false;
         auto  ft    = lua_libraries::form_types::pull(L, 1, valid);
         if (!valid)
            return 0;
         auto& info = dovah::form_type_info::lookup(ft);
         if (info.flags & dovah::form_type_info::flag::is_singleton) {
            lua_pushinteger(L, 1);
            return 1;
         }
         lua_pushinteger(L, editor.count_forms_of_type(ft));
         return 1;
      }
      int create_form(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& editor = DovahKitCore::get();
         if (!editor.has_data())
            cobb::lua::error(L, "cannot create a new form because no data is loaded in the editor");
         bool  valid = false;
         auto  ft    = dovahscript::lua_libraries::form_types::pull(L, 1, valid);
         if (!valid)
            cobb::lua::error(L, "cannot create a new form because no valid form type was supplied");
            
         auto* m = new tasks::s2m::create_form;
         m->form_type = ft;
         if (lua_gettop(L) > 1 && lua_type(L, 2) == LUA_TTABLE) { // if an options table was passed
            lua_settop(L, 2);
            //
            lua_getfield(L, 2, "parent");
            if (!lua_isnoneornil(L, 3)) {
               auto* wrap = (wrapper*) classes::cast_to_class(L, 3, wrappers::form::metatable_key);
               if (wrap) {
                  m->parent = wrap->stub;
               } else {
                  lua_warning(L, "dovah.create_form() call tried to specify a parent but didn't pass a form", 0);
               }
            }
            lua_settop(L, 2);
            //
            lua_getfield(L, 2, "grid_coordinates");
            if (!lua_isnoneornil(L, 3)) {
               if (lua_type(L, 3) == LUA_TTABLE) {
                  lua_getfield(L, 3, "x");
                  lua_getfield(L, 3, "y");
                  m->cell_grid_coordinates.x = lua_tonumber(L, 4);
                  m->cell_grid_coordinates.y = lua_tonumber(L, 5);
               } else {
                  lua_warning(L, "dovah.create_form() call tried to specify grid coordinates for an exterior cell, but didn't pass valid numbers", 0);
               }
            }
            lua_settop(L, 2);
            //
            lua_getfield(L, 2, "editor_id");
            if (!lua_isnoneornil(L, 3)) {
               m->editorID = luaL_tolstring(L, 3, nullptr);
            }
            lua_settop(L, 2);
         }
         send_script_task(*m);
         if (m->error) {
            cobb::lua::error(L, m->error_text.c_str());
         }
         auto* stub = m->result;
         delete m;
         //
         return push_native_object(stub);
      }
      int deep_stringify(lua_State* L) {
         lua_settop(L, 1);
         auto string = cobb::lua::var_to_string(L, 1);
         lua_pushlstring(L, string.c_str(), string.size());
         return 1;
      }
      int dump(lua_State* L) {
         lua_settop(L, 1);
         auto string = cobb::lua::var_to_string(L, 1);
         //
         auto m = new tasks::s2m::log_message();
         m->text = QString::fromStdString(string);
         send_script_task(*m);
         return 0;
      }
      int for_each_form_of_type(lua_State* L) {
         luaL_argcheck(L, lua_isfunction(L, 2), 2, "function expected");
         auto& editor = DovahKitCore::get();
         if (!editor.has_data())
            return 0;
         //
         bool  valid = false;
         auto  ft    = lua_libraries::form_types::pull(L, 1, valid);
         if (!valid)
            return 0;
         auto& info  = dovah::form_type_info::lookup(ft);
         if (info.flags & dovah::form_type_info::flag::is_singleton) {
            //
            // For singleton forms, only use the canonical stub.
            //
            auto* stub = editor.get_singleton_form(ft, false);
            if (stub) {
               lua_pushvalue(L, 2); // push the function
               int argcount = push_native_object(stub);
               if (argcount > 0) {
                  if (argcount > 1)
                     lua_pop(L, argcount - 1);
                  lua_call(L, 1, 1);
               } else {
                  lua_pop(L, 1);
               }
            }
            return 0;
         }
         //
         editor.for_each_form_of_type(ft, [L](dovah::form_stub* stub) {
            lua_pushvalue(L, 2); // push the function
            int argcount = push_native_object(stub);
            if (argcount > 0) {
               if (argcount > 1)
                  lua_pop(L, argcount - 1);
               lua_call(L, 1, 1);
               if (lua_toboolean(L, -1) == 1)
                  return true;
               lua_pop(L, 1);
            } else {
               lua_pop(L, 1);
            }
            return false;
         });
         //
         return 0;
      }
      int get_form_by_editor_id(lua_State* L) {
         cobb::lua::argcheck(L, lua_isstring(L, 1), 1, "string expected");
         std::string_view editor_id = lua_tostring(L, 1);
         cobb::lua::argcheck(L, !editor_id.empty(), 1, "cannot search for an empty string");

         auto& editor = DovahKitCore::get();
         if (!editor.has_data())
            return 0;

         dovah::form_stub* found = nullptr;
         {
            bool  valid = false;
            auto  ft    = lua_libraries::form_types::pull(L, 2, valid);
            if (valid) {
               editor.for_each_form_of_type(ft, [&editor_id, &found](dovah::form_stub* form) -> bool {
                  if (form->editorID == editor_id) {
                     found = form;
                     return true;
                  }
                  return false;
               });
            } else {
               editor.for_each_form([&editor_id, &found](dovah::form_stub* form) -> bool {
                  if (form->editorID == editor_id) {
                     found = form;
                     return true;
                  }
                  return false;
               });
            }
         }
         if (!found)
            return 0;
         return push_native_object(found);
      }
      int get_form_by_id(lua_State* L) {
         luaL_argcheck(L, lua_isnumber(L, 1), 1, "form ID (number) expected");
         auto& editor = DovahKitCore::get();
         if (!editor.has_data())
            return 0;
         int  isnum;
         auto id = lua_tointegerx(L, 1, &isnum);
         if (!isnum)
            return 0;
         if (id < 0 || id > 0xFFFFFFFF)
            return 0;
         auto* stub = editor.get_form(id);
         if (!stub)
            return 0;
         //
         return push_native_object(stub);
      }
      int log_message(lua_State* L) {
         auto m = new tasks::s2m::log_message();
         //
         auto argcount = lua_gettop(L);
         if (!argcount)
            return 0;
         //
         if (lua_type(L, 1) != LUA_TSTRING) { // coerce argument 1 to a string if it isn't one, as string.format doesn't do this automatically
            luaL_tolstring(L, 1, nullptr);
            lua_copy(L, argcount + 1, 1);
            lua_pop(L, 1);
         }
         //
         lua_getfield(L, LUA_REGISTRYINDEX, string_format_registry_key);
         if (lua_isfunction(L, argcount + 1)) {
            lua_rotate(L, 1, 1); // move (string.format) ahead of the other stack elements
            lua_call  (L, argcount, 1);
         }
         //
         const char* out = lua_tostring(L, 1);
         if (!out) {
            out = "";
         }
         m->text = QString::fromUtf8(out);
         //
         send_script_task(*m);
         return 0;
      }
      int load_game_asset(lua_State* L) {
         auto params = api_helpers::pull_load_resource_params(L, 1);
         params.load_from = decltype(params)::source::game_assets;
         return api_helpers::load_and_push_resource(L, params);
      }
      int lookup_game_ini_setting(lua_State* L) {
         QString filename;
         QString category;
         QString setting;
         if (lua_isstring(L, 1)) {
            luaL_argcheck(L, lua_isstring(L, 2), 2, "string (category name) expected");
            luaL_argcheck(L, lua_isstring(L, 3), 3, "string (setting name) expected");
            filename = QString::fromUtf8(lua_tostring(L, 1));
            category = QString::fromUtf8(lua_tostring(L, 2));
            setting  = QString::fromUtf8(lua_tostring(L, 3));
         } else {
            lua_settop(L, 1);
            if (!lua_istable(L, 1) && !lua_isuserdata(L, 1))
               cobb::lua::argerror(L, 1, "expected three strings (file, category, and setting names), or a table or userdata with the same info");
            lua_getfield(L, 1, "filename");
            lua_getfield(L, 1, "category");
            lua_getfield(L, 1, "setting");
            if (!lua_isstring(L, 2))
               cobb::lua::argerror(L, 1, "argument.filename was not a string");
            if (!lua_isstring(L, 3))
               cobb::lua::argerror(L, 1, "argument.category was not a string");
            if (!lua_isstring(L, 4))
               cobb::lua::argerror(L, 1, "argument.setting was not a string");
            filename = QString::fromUtf8(lua_tostring(L, 2));
            category = QString::fromUtf8(lua_tostring(L, 3));
            setting  = QString::fromUtf8(lua_tostring(L, 4));
            lua_pop(L, 3);
         }
         //
         cobb::qt::ini::File* file = nullptr;
         if (filename.endsWith(".ini", Qt::CaseInsensitive))
            filename.chop(4);
         if (filename.compare("Skyrim", Qt::CaseInsensitive) == 0) {
            file = &editor::game_inis::get_skyrim();
         } else if (filename.compare("SkyrimPrefs", Qt::CaseInsensitive) == 0) {
            file = &editor::game_inis::get_skyrim_prefs();
         } else {
            cobb::lua::error(L, "filename '%s' is not a known game INI file", filename.toUtf8());
         }
         //
         cobb::qt::ini::Setting* result = nullptr;
         {
            auto* task = new tasks::s2m::lambda(true);
            task->handler = [file, &category, &setting, &result]() {
               result = file->setting(category, setting);
            };
            send_script_task(*task);
            delete task;
         }
         if (!result)
            return 0;
         return push_native_object(result);
      }
      int object_is(lua_State* L) {
         struct stack_offset {
            enum {
               arg_subject     = 1,
               arg_classname   = 2,
               class_metatable = 3,
               class_list      = 4,
               class_list_item = 5,
               class_name      = 6,
            };
         };
         lua_settop(L, 2);
         luaL_argcheck(L, lua_isstring(L, stack_offset::arg_classname), stack_offset::arg_classname, "typename (string) expected");
         auto  t   = lua_type    (L, stack_offset::arg_subject);
         auto* req = lua_tostring(L, stack_offset::arg_classname);
         if (t == LUA_TTABLE || t == LUA_TUSERDATA) {
            lua_getmetatable(L, stack_offset::arg_subject);
            assert(lua_gettop(L) == stack_offset::class_metatable);
            lua_pushstring(L, "__classlist");
            lua_rawget(L, stack_offset::class_metatable);
            if (!lua_istable(L, stack_offset::class_list)) {
               lua_pushboolean(L, false);
               return 1;
            }
            auto len = lua_rawlen(L, stack_offset::class_list);
            for (decltype(len) i = 1; i <= len; ++i) {
               lua_rawgeti(L, stack_offset::class_list, i);
               lua_pushstring(L, "__name");
               if (lua_rawget(L, stack_offset::class_list_item) == LUA_TSTRING) {
                  if (lua_rawequal(L, stack_offset::arg_classname, stack_offset::class_name)) {
                     lua_pushboolean(L, true);
                     return 1;
                  }
               }
               lua_settop(L, stack_offset::class_list);
            }
         }
         const char* tn  = lua_typename(L, t);
         lua_pushboolean(L, strcmp(tn, req) == 0);
         return 1;
      }
      int type(lua_State* L) {
         lua_settop(L, 1);
         lua_checkstack(L, 3);
         auto t = lua_type(L, 1);
         if (t == LUA_TTABLE || t == LUA_TUSERDATA) {
            if (lua_getmetatable(L, 1)) {
               lua_pushstring(L, "__name");
               auto nt = lua_rawget(L, 2);
               if (nt == LUA_TSTRING)
                  return 1;
            }
         }
         lua_pushstring(L, lua_typename(L, t));
         return 1;
      }
   }

   namespace _member_constructors {
      namespace impl::version {
         int __index(lua_State* L) { // function index(t, k)
            lua_getmetatable(L, 1);              // [t, k, meta]
            lua_getfield    (L, 3, "__version"); // [t, k, meta, meta.__version]
            lua_copy        (L, 4, 1);           // [meta, k, meta, meta.__version]
            lua_settop      (L, 2);              // [meta, k]
            lua_rawget(L, 1); // [meta, meta[k]]
            return 1;
         }
         int __tostring(lua_State* L) {
            auto out = QString("%1.%2.%3.%4");
            for (int i = 0; i < 4; ++i) {
               lua_rawgeti(L, 1, i + 1);
               out = out.arg(lua_tointeger(L, -1));
               lua_pop(L, 1);
            }
            lua_pushstring(L, out.toUtf8());
            return 1;
         }
      }
      int version(lua_State* L) {
         static constexpr const std::array field_names = { "major", "minor", "patch", "build" };
         //
         lua_createtable(L, 4, 4);
         auto v     = QApplication::applicationVersion();
         bool empty = v.isEmpty();
         if (!empty) {
            auto m = QRegularExpression(R"(^(\d+)\.(\d+)\.(\d+)\.(\d+)$)").match(v);
            if (m.hasMatch()) {
               for (size_t i = 0; i < field_names.size(); ++i) {
                  auto v = m.capturedView(i + 1).toInt();
                  //
                  lua_pushinteger(L, v);
                  lua_setfield(L, -2, field_names[i]);
                  lua_pushinteger(L, v);
                  lua_rawseti(L, -2, i + 1);
               }
            } else {
               empty = true;
            }
         }
         if (empty) {
            for (size_t i = 0; i < field_names.size(); ++i) {
               lua_pushnil(L);
               lua_setfield(L, -2, field_names[i]);
               lua_pushnil(L);
               lua_rawseti(L, -2, i + 1);
            }
         }
         //
         lua_createtable(L, 0, 2); // metatable
         //
         lua_pushcfunction(L, &impl::version::__index);
         lua_setfield(L, -2, "__index");
         lua_pushcfunction(L, &impl::version::__tostring);
         lua_setfield(L, -2, "__tostring");
         //
         lua_setmetatable(L, -2);
         return 1;
      }
   }
   
   const std::initializer_list<luaL_Reg> _functions = {
      luaL_Reg{ "count_forms_of_type",     &_definitions::count_forms_of_type },
      luaL_Reg{ "create_form",             &_definitions::create_form },
      luaL_Reg{ "deep_stringify",          &_definitions::deep_stringify },
      luaL_Reg{ "dump",                    &_definitions::dump },
      luaL_Reg{ "for_each_form_of_type",   &_definitions::for_each_form_of_type },
      luaL_Reg{ "get_form_by_editor_id",   &_definitions::get_form_by_editor_id },
      luaL_Reg{ "get_form_by_id",          &_definitions::get_form_by_id },
      luaL_Reg{ "log_message",             &_definitions::log_message },
      luaL_Reg{ "load_game_asset",         &_definitions::load_game_asset },
      luaL_Reg{ "lookup_game_ini_setting", &_definitions::lookup_game_ini_setting},
      luaL_Reg{ "object_is",               &_definitions::object_is },
      luaL_Reg{ "type",                    &_definitions::type },
   };
   const std::initializer_list<luaL_Reg> _members = {
      luaL_Reg{ "version", &_member_constructors::version },
   };
}

namespace dovahscript::lua_libraries {
   namespace dovah {
      extern void import(lua_State* L) {
         lua_getglobal(L, "string");
         lua_getfield (L, -1, "format");
         lua_setfield (L, LUA_REGISTRYINDEX, string_format_registry_key);
         lua_pop      (L, 1);
         //
         lua_createtable(L, 0, _functions.size() + _members.size());
         int tbl = lua_gettop(L);
         cobb::lua::setfuncs(L, _functions);
         for (auto& pair : _members) {
            int count = (pair.func)(L);
            if (count <= 0)
               continue;
            if (count == 1) {
               lua_setfield(L, tbl, pair.name);
            } else {
               lua_createtable(L, count, 0);
               int t = lua_gettop(L);
               for (int i = 0; i < count; ++i) {
                  lua_pushvalue(L, i - t);
                  lua_rawseti(L, -2, i + 1);
               }
               lua_setfield(L, tbl, pair.name);
               lua_pop(L, count);
            }
         }
         lua_setglobal(L, "dovah");
      }
   }
}