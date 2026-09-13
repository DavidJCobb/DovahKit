#include "form.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../send_script_task.h"

#include "../../wrapper.h"
#include "../../core/classes.h"
#include "../../../dovah/forms/Form.h"
#include "../../push_native_object.h"
#include "../../lua_libraries/form_types.h"
#include "dovahscript/api_helpers/fail_if_form_cannot_be_edited.h"

#include "../../tasks/s2m/delete_form.h"
#include "../../tasks/s2m/duplicate_form.h"
#include "../../tasks/s2m/renumber_form.h"

#include "../../../dovah/files/tes_file_reading/file_loader.h"

#include "../form_components/papyrus.h"

namespace {
   using namespace dovahscript;
   using cls = wrappers::form;
   //
   bool _stub_source_file_to_table(lua_State* L, const dovah::form_stub::owner_file_t* file) {
      if (file->header.details & dovah::tes_file_header::detail_flag::is_hardcoded_dummy)
         return false;
      if (file->header.details & dovah::tes_file_header::detail_flag::is_none_stub_dummy)
         return false;
      lua_createtable(L, 0, 0);
      //
      lua_pushstring (L, file->get_filename().c_str());
      lua_setfield   (L, -2, "filename");
      lua_pushboolean(L, file->is_master());
      lua_setfield   (L, -2, "is_master");
      lua_pushboolean(L, file->is_light());
      lua_setfield   (L, -2, "is_light");
      //
      return true;
   }
   //
   namespace _methods {
      int delete_(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* m = new tasks::s2m::delete_form;
         m->stub = self.stub;
         send_script_task(*m);
         if (m->results.failed) {
            if (!m->results.text)
               m->results.text = "";
            cobb::lua::error(L, m->results.text);
         }
         delete m;
         return 0;
      }
      int duplicate(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         // TODO: accept an optional second argument consisting of a table with named options, such 
         // as the editor ID to use for the duplicate, a different parent cell to use for the duplicate, 
         // and so on.
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* m = new tasks::s2m::duplicate_form;
         m->source = self.stub;
         if (lua_gettop(L) > 1 && lua_type(L, 2) == LUA_TTABLE) { // if an options table was passed
            lua_settop(L, 2);
            //
            lua_getfield(L, 2, "parent");
            if (!lua_isnoneornil(L, 3)) {
               auto* wrap = (wrapper*) dovahscript::classes::cast_to_class(L, 3, wrappers::form::metatable_key);
               if (wrap) {
                  m->parent = wrap->stub;
               } else {
                  lua_warning(L, "form:duplicate() call tried to specify a parent but didn't pass a form", 0);
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
                  lua_warning(L, "form:duplicate() call tried to specify grid coordinates for an exterior cell, but didn't pass valid numbers", 0);
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
      int form_id_to_string(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub) {
            lua_pushstring(L, "00000000");
            return 1;
         }
         auto  formID = self.stub->formID;
         char  str[9] = "00000000";
         char* digit  = &str[7];
         for (; digit >= str; --digit, formID >>= 0x4) {
            auto d = formID & 0xF;
            if (d < 0xA)
               *digit = ('0' + d);
            else
               *digit = ('A' + d - 0xA);
         }
         lua_pushstring(L, str);
         return 1;
      }
      int get_last_source_file(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub) {
            lua_pushnil(L);
            return 1;
         }
         auto* stub = self.stub;
         auto* file = stub->get_file_at_index(-1);
         if (!file) {
            lua_pushnil(L);
            return 1;
         }
         if (_stub_source_file_to_table(L, file))
            return 1;
         lua_pushnil(L);
         return 1;
      }
      int get_source_file_list(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* stub = self.stub;
         auto  size = stub->source_file_count();
         lua_createtable(L, size, 0);
         auto  tbli = lua_gettop(L);
         int   j    = 1;
         for (decltype(size) i = 0; i < size; ++i) {
            const auto* file = stub->get_file_at_index(i);
            if (_stub_source_file_to_table(L, file)) {
               lua_rawseti(L, tbli, j);
               ++j;
            }
         }
         return 1;
      }
      int get_user_forms(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         //
         auto* stub  = self.stub;
         lua_createtable(L, stub->inbound.size(), 0);
         auto  table = lua_gettop(L);
         //
         int   i  = 0; // Lua arrays start with 1, but we ++increment the index before using it, so this is fine
         for (auto& pair : stub->inbound) {
            auto& entry = pair.second;
            if (!entry.other)
               continue;
            int wcount = push_native_object(entry.other);
            while (wcount--)
               lua_seti(L, table, ++i);
         }
         //
         return 1;
      }
   }
   namespace _getters {
      int editor_id(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         lua_pushstring(L, self.stub->get_editor_id());
         return 1;
      }
      int flags(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         lua_pushinteger(L, self.stub->get_record_flags());
         return 1;
      }
      int form_id(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         lua_pushinteger(L, self.stub->formID);
         return 1;
      }
      int form_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         lua_libraries::form_types::push(L, self.stub->form_type);
         return 1;
      }
      int papyrus(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         if (!form)
            return 0;
         return wrappers::papyrus_root::wrap_and_push(L, form->stub);
      }
   }
   namespace _setters {
      int editor_id(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "editor ID (string) expected");
         if (!self.stub)
            return 0;
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         //
         auto editorID = lua_tostring(L, 2);
         if (self.stub->editorID == editorID)
            return 0;
         //
         self.before_edit();
         self.stub->editorID = editorID;
         self.after_edit();
         //
         return 0;
      }
      int form_id(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_type(L, 2) == LUA_TNUMBER, 2, "form ID (number) expected");
         if (!self.stub)
            return 0;
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         //
         int result;
         dovah::bare_form_id_t formID = lua_tointegerx(L, 2, &result);
         luaL_argcheck(L, result, 2, "form ID must be an integer");
         //
         if (formID == self.stub->formID) // (form.form_id = form.form_id) should be treated as a no-op
            return 0;
         //
         auto* m = new tasks::s2m::renumber_form;
         m->stub      = self.stub;
         m->desiredID = formID;
         send_script_task(*m);
         if (m->error) {
            if (!m->error_text)
               m->error_text = "";
            cobb::lua::error(L, m->error_text);
         }
         delete m;
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "delete",               &_methods::delete_ },
      { "duplicate",            &_methods::duplicate },
      { "form_id_to_string",    &_methods::form_id_to_string },
      { "get_last_source_file", &_methods::get_last_source_file },
      { "get_source_file_list", &_methods::get_source_file_list },
      { "get_user_forms",       &_methods::get_user_forms },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "editor_id", &_getters::editor_id },
      { "flags",     &_getters::flags },
      { "form_id",   &_getters::form_id },
      { "form_type", &_getters::form_type },
      { "papyrus",   &_getters::papyrus },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "editor_id", &_setters::editor_id },
      { "form_id",   &_setters::form_id },
   };
}