#include "dovah.h"
#include "../../editor_script_core.h"
#include "../../util.h"
#include "../../wrapper_util.h"
#include "../../classes/benchmark.h"

#include "../../../core.h" // DovahKitCore
#include "../form_type_values.h"
#include "../../cross_thread_tasks/s2m/create_form.h"
#include "../../cross_thread_tasks/s2m/log_message.h"
#include "../../cross_thread_tasks/s2m/test_call_and_response.h"
#include "../../wrappers/form.h"

namespace {
   using namespace editor_script;

   namespace _definitions {
      luastackchange_t benchmark_start(lua_State* L) {
         auto* p = lua_newuserdata(L, sizeof(classes::benchmark)); // push 1
         luaL_getmetatable(L, classes::benchmark::metatable_key); // push 1
         lua_setmetatable(L, -2); // pop 1
         new (p) classes::benchmark;
         return 1;
      }
      luastackchange_t benchmark_stop(lua_State* L) {
         auto* self = (classes::benchmark*) editor_script::cast_to_exact_class(L, 1, classes::benchmark::metatable_key);
         if (self == nullptr) {
            luaL_error(L, "bad argument #1 to dovah.benchmark_stop (expected %s)", classes::benchmark::metatable_key);
         }
         __assume(self != nullptr);
         self->finish();
         return 0;
      }
      luastackchange_t count_forms_of_type(lua_State* L) {
         luaL_argcheck(L, lua_isnumber(L, 1), 1, "form type (number) expected");
         auto& editor = DovahKitCore::get();
         if (!editor.has_data()) {
            lua_pushinteger(L, 0);
            return 1;
         }
         //
         bool  valid = false;
         auto  ft    = editor_script::get_form_type_from_stack(L, 1, valid);
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
      luastackchange_t create_form(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         luaL_argcheck(L, lua_isnumber(L, 1), 1, "form type (number) expected");
         auto& editor = DovahKitCore::get();
         if (!editor.has_data())
            luaL_error(L, "cannot create a new form because no data is loaded in the editor");
         bool  valid = false;
         auto  ft    = editor_script::get_form_type_from_stack(L, 1, valid);
         if (!valid)
            luaL_error(L, "cannot create a new form because no valid form type was supplied");
            
         auto* m = new tasks::s2m::create_form;
         m->form_type = ft;
         if (lua_gettop(L) > 1 && lua_type(L, 2) == LUA_TTABLE) { // if an options table was passed
            lua_settop(L, 2);
            //
            lua_getfield(L, 2, "parent");
            if (!lua_isnoneornil(L, 3)) {
               auto* wrap = (wrapper*)editor_script::cast_to_class(L, 3, wrappers::form::metatable_key);
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
         DovahKitScriptVMMessenger::get().send_message(m);
         if (m->error) {
            if (!m->error_text)
               m->error_text = "";
            luaL_error(L, m->error_text);
         }
         auto* stub = m->result;
         delete m;
         //
         wrapper out;
         auto* mt = wrap_form(out, stub);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t for_each_form_of_type(lua_State* L) {
         luaL_argcheck(L, lua_isnumber(L, 1),   1, "form type (number) expected");
         luaL_argcheck(L, lua_isfunction(L, 2), 2, "function expected");
         auto& editor = DovahKitCore::get();
         if (!editor.has_data())
            return 0;
         //
         bool  valid = false;
         auto  ft    = editor_script::get_form_type_from_stack(L, 1, valid);
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
               wrapper out;
               auto*   mt = wrap_form(out, stub);
               if (DovahKitScriptVMUserdataInterface::get().push(L, out, mt))
                  lua_call(L, 1, 1);
            }
            return 0;
         }
         //
         editor.for_each_form_of_type(ft, [L](dovah::form_stub* stub) {
            lua_pushvalue(L, 2); // push the function
            wrapper out;
            auto*   mt = wrap_form(out, stub);
            if (DovahKitScriptVMUserdataInterface::get().push(L, out, mt)) {
               lua_call(L, 1, 1);
               if (lua_toboolean(L, -1) == 1) {
                  return true;
               }
            } else {
               lua_settop(L, 2);
            }
            return false;
         });
         //
         return 0;
      }
      luastackchange_t get_form_by_id(lua_State* L) {
         luaL_argcheck(L, lua_isnumber(L, 1), 1, "form ID (number) expected");
         auto& editor = DovahKitCore::get();
         if (!editor.has_data())
            return 0;
         auto  id   = lua_tonumber(L, 1);
         auto* stub = editor.get_form(id);
         if (!stub)
            return 0;
         //
         wrapper out;
         auto*   mt = wrap_form(out, stub);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t log_message(lua_State* L) {
         auto m = new editor_script::tasks::s2m::log_message();
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
         lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVM::string_format_registry_key);
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
         DovahKitScriptVMMessenger::get().send_message(m);
         return 0;
      }
      luastackchange_t test_call_and_response(lua_State* L) {
         auto* m = new editor_script::tasks::s2m::test_call_and_response();
         DovahKitScriptVMMessenger::get().send_message(m);
         return 0;
      }
   }

   std::array _functions = {
      luaL_Reg{ "benchmark_start",        &_definitions::benchmark_start },
      luaL_Reg{ "benchmark_stop",         &_definitions::benchmark_stop },
      luaL_Reg{ "count_forms_of_type",    &_definitions::count_forms_of_type },
      luaL_Reg{ "create_form",            &_definitions::create_form },
      luaL_Reg{ "for_each_form_of_type",  &_definitions::for_each_form_of_type },
      luaL_Reg{ "get_form_by_id",         &_definitions::get_form_by_id },
      luaL_Reg{ "log_message",            &_definitions::log_message },
      luaL_Reg{ "test_call_and_response", &_definitions::test_call_and_response },
   };
}
namespace editor_script::namespace_setup {
   extern void dovah(lua_State* L) {
      int pos = lua_gettop(L);
      for (auto& entry : _functions) {
         lua_pushstring(L, entry.name);    // key
         lua_pushcfunction(L, entry.func); // value
         lua_rawset(L, pos);
      }
   }
}
