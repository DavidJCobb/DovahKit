#include "formlist.h"
#include "../classes.h"
#include "../util.h"
#include "../editor_script_core.h"
#include "../wrapper_util.h"
#include "../collections.h"

namespace {
   using namespace editor_script;
   using _wrapper_t     = wrappers::formlist;
   using _loaded_form_t = dovah::loaded_forms::FormList;
}

#pragma region Collection: "entries"
namespace {
   using namespace editor_script;

   namespace _collections::entries {
      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, _wrapper_t::entry_collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", _wrapper_t::entry_collection_key);
         }
         return *self;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushinteger(L, form->contents.size());
         return 1;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         auto  i    = lua_tointeger(L, 2);
         auto& list = form->contents;
         if (i > list.size() || i <= 0)
            return 0;
         --i;
         wrapper out;
         auto* mt = wrap_form(out, list[i].get_form_stub());
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t member_function_insert(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_collection_wrapper(L);
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
         //
         int  pos_value = 2;
         bool has_index = false;
         //
         if (lua_gettop(L) >= 3) {
            has_index = true;
            pos_value = 3;
            luaL_argcheck(L, lua_isinteger(L, 2), 2, "provided index is not an integer");
         }
         dovah::form_stub* target = nullptr;
         if (!lua_isnoneornil(L, pos_value)) {
            auto* w = wrapper_from_stack<wrappers::form>(L, pos_value);
            if (!w)
               return luaL_error(L, "you can only insert forms or nil into a formlist");
            target = w->stub;
         }
         //
         if (!form)
            return 0;
         auto& list = form->contents;
         auto  size = list.size();
         int   i    = size + 1;
         if (has_index) {
            i = lua_tointeger(L, 2);
            if (i < 1)
               return luaL_error(L, "indices below 1, such as %d, are not allowed", i);
            --i;
         }
         if (i >= size) {
            if (i > size) {
               lua_warning(L, "index ", 1);
               lua_warning(L, std::to_string(i).c_str(), 1);
               lua_warning(L, " is out of bounds; nil elements will be created between the end of the list and the new element", 0);
            }
            list.resize(i + 1);
         } else {
            list.emplace(list.begin() + i);
         }
         self.before_edit();
         list[i].set(*form, target);
         self.after_edit();
         return 0;
      }
      luastackchange_t member_function_remove(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected an integer index");
         int isnum;
         int i = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "expected an integer index");
         if (!form)
            return 0;
         auto& list = form->contents;
         if (i > list.size() || i <= 0)
            return 0;
         --i;
         self.before_edit();
         list[i].set(*form, nullptr);
         list.erase(list.begin() + i);
         self.after_edit();
         return 0;
      }
      luastackchange_t set_item(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         constexpr auto index_self  = 1;
         constexpr auto index_key   = 2;
         constexpr auto index_value = 3;
         //
         dovah::form_stub* target = nullptr;
         if (!lua_isnoneornil(L, index_value)) {
            auto* w = wrapper_from_stack<wrappers::form>(L, index_value);
            if (!w)
               return luaL_error(L, "you can only overwrite formlist entries with forms or nil");
            target = w->stub;
         }
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         int v = 0;
         int i = lua_tointegerx(L, index_key, &v);
         if (!v)
            return luaL_error(L, "indices in a formlist's entry list must be integers");
         if (i < 1)
            return luaL_error(L, "indices below 1, such as %d, are not allowed", i);
         --i;
         //
         auto& list = form->contents;
         auto  size = list.size();
         if (i >= size) {
            if (i > size) {
               lua_warning(L, "index ", 1);
               const char* tostr = lua_tolstring(L, index_key, nullptr);
               lua_warning(L, tostr, 1);
               lua_warning(L, " is out of bounds; nil elements will be created between the end of the list and the new element", 0);
            }
            list.resize(i + 1);
         }
         self.before_edit();
         list[i].set(*form, target);
         self.after_edit();
         return 0;
      }
   }
}
#pragma endregion

#pragma region form
namespace {
   namespace _getters {
      luastackchange_t entries(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(editor_script::wrapper_part_types::formlist_entries);
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, _wrapper_t::entry_collection_key);
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_methods = no_functions;
   
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_getters = {
      { "entries", &_getters::entries },
   };
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_setters = no_functions;

   /*static*/ void _wrapper_t::build_collection_metatables(lua_State* L) {
      define_collection_metatable(L, {
         .registry_key          = _wrapper_t::entry_collection_key,
         .garbage_collection    = &wrapper::__gc,
         //
         .get_collection_length  = &_collections::entries::get_collection_length,
         .lookup_item_by_index   = &_collections::entries::lookup_item_by_index,
         .member_function_insert = &_collections::entries::member_function_insert,
         .member_function_remove = &_collections::entries::member_function_remove,
         .set_item               = &_collections::entries::set_item,
      });
   }
}
#pragma endregion