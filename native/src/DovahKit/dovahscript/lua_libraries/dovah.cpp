#include "dovah.h"
#include <cassert>
#include "../../../helpers/lua/dump.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/setfuncs.h"
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
#include "../../../editor/core.h"

#include "../../../dovah/data/ini_settings.h"

#include "../lua_classes/benchmark.h"

#include "../../editor/subsystems/game_inis.h"
#include "../wrappers/ini/setting.h"

// For resources:
#include <QBuffer>
#include <QImage>
#include <QImageReader>
#include "../../dovah/files/bsa/bsa_archived_file.h"
#include "../wrappers/resource/dds.h"
#include "../wrappers/resource/unknown.h"

namespace {
   static constexpr const char* string_format_registry_key = "dovahscript.internal.dovah.string_format_copy";
}

namespace {
   using namespace dovahscript;
   
   namespace _definitions {
      int benchmark_start(lua_State* L) {
         lua_classes::benchmark::push_new_instance(L);
         return 1;
      }
      int benchmark_stop(lua_State* L) {
         auto* self = (lua_classes::benchmark*) classes::cast_to_exact_class(L, 1, lua_classes::benchmark::metatable_key);
         cobb::lua::argcheck(L, self != nullptr, 1, "expected benchmark object");
         self->finish();
         return 0;
      }
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
            if (!m->error_text)
               m->error_text = "";
            cobb::lua::error(L, m->error_text);
         }
         auto* stub = m->result;
         delete m;
         //
         return push_native_object(stub);
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
      int lookup_game_asset(lua_State* L) {
         const char* raw = nullptr;
         if (lua_isstring(L, 1)) {
            raw = lua_tostring(L, 1);
         } else if (lua_istable(L, 1) || lua_isuserdata(L, 1)) {
            int type = luaL_getmetafield(L, 1, "__tostring");
            lua_pop(L, 1);
            if (type == LUA_TFUNCTION)
               raw = luaL_tolstring(L, 1, nullptr);
         }
         luaL_argcheck(L, raw != nullptr, 1, "string expected");
         std::filesystem::path path = raw;
         //
         std::unique_ptr<dovah::bsa_archived_file> file = nullptr;
         {
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [path, &file]() {
               file.reset(DovahKitCore::get().lookup_game_asset(path, true));
            };
            send_script_task(*task);
            delete task;
         }
         if (!file)
            return 0;
         {
            QImageReader reader;
            {
               auto p = path.filename().u8string();
               auto s = QString::fromUtf8((const char*)p.c_str());
               reader.setFileName(s);
            }
            auto buffer = QByteArray::fromRawData((const char*)file->data(), file->size());
            auto device = QBuffer(&buffer);
            reader.setDevice(&device);
            if (!reader.format().isEmpty()) {
               auto raster = reader.read();
               if (!raster.isNull()) {
                  DovahscriptResourceHandle resource;
                  {
                     auto* task    = new tasks::s2m::lambda(true);
                     task->handler = [&raster, &resource]() {
                        resource = core::subsystems::resources::get().create_resource(raster);
                     };
                     send_script_task(*task);
                     delete task;
                     assert(resource);
                  }
                  return push_native_object(resource);
               }
            }
         }
         if (_stricmp(path.extension().string().data(), ".dds") == 0) {
            DovahscriptResourceHandle resource;
            {
               auto* task    = new tasks::s2m::lambda(true);
               task->handler = [&file, &resource]() {
                  resource = core::subsystems::resources::get().create_resource(QByteArray::fromRawData((const char*)file->data(), file->size()), resource_type::dds);
               };
               send_script_task(*task);
               delete task;
            }
            return push_native_object(resource);
         }
         //
         // The resource could not be identified.
         //
         DovahscriptResourceHandle resource = nullptr;
         {
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [&file, &resource]() {
               resource = core::subsystems::resources::get().create_resource(QByteArray::fromRawData((const char*)file->data(), file->size()));
            };
            send_script_task(*task);
            delete task;
            assert(resource);
         }
         return push_native_object(resource);
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
         lua_settop(L, 2);
         constexpr int index_obj = 1;
         constexpr int index_req = 2;
         constexpr int index_mt  = 3;
         //
         luaL_argcheck(L, lua_isstring(L, index_req), 2, "typename (string) expected");
         auto  t   = lua_type    (L, index_obj);
         auto* req = lua_tostring(L, index_req);
         if (t == LUA_TTABLE || t == LUA_TUSERDATA) {
            lua_getmetatable(L, index_obj);
            assert(lua_gettop(L) == index_mt);
            while (lua_type(L, index_mt) == LUA_TTABLE) {
               lua_pushstring(L, "__name");
               if (lua_rawget(L, index_mt) == LUA_TSTRING) {
                  auto* tn = lua_tostring(L, -1);
                  if (strcmp(tn, req) == 0) {
                     lua_pushboolean(L, true);
                     return 1;
                  }
               }
               lua_settop(L, index_mt);
               lua_pushstring(L, "__superclass");
               lua_rawget    (L, index_mt);
               lua_replace(L, index_mt);
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
   
   const std::initializer_list<luaL_Reg> _functions = {
      luaL_Reg{ "benchmark_start",         &_definitions::benchmark_start },
      luaL_Reg{ "benchmark_stop",          &_definitions::benchmark_stop },
      luaL_Reg{ "count_forms_of_type",     &_definitions::count_forms_of_type },
      luaL_Reg{ "create_form",             &_definitions::create_form },
      luaL_Reg{ "dump",                    &_definitions::dump },
      luaL_Reg{ "for_each_form_of_type",   &_definitions::for_each_form_of_type },
      luaL_Reg{ "get_form_by_id",          &_definitions::get_form_by_id },
      luaL_Reg{ "log_message",             &_definitions::log_message },
      luaL_Reg{ "lookup_game_asset",       &_definitions::lookup_game_asset },
      luaL_Reg{ "lookup_game_ini_setting", &_definitions::lookup_game_ini_setting},
      luaL_Reg{ "object_is",               &_definitions::object_is },
      luaL_Reg{ "type",                    &_definitions::type },
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
         lua_createtable(L, 0, _functions.size());
         cobb::lua::setfuncs(L, _functions);
         lua_setglobal(L, "dovah");
      }
   }
}