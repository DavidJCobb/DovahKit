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
         int  v = 0;
         auto i = lua_tointegerx(L, index_key, &v);
         if (!v)
            return luaL_error(L, "indices in a formlist's entry list must be integers");
         if (i < 1)
            return luaL_error(L, "index %d is out of bounds", i);
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
         list[i].set(*form->stub, target);
         if (!target && i == size - 1) {
            //
            // TODO: Setting a FormList entry to (nil) should remove the entry, shortening the 
            //       list. We can define a special (no_form) constant (a userdata) for scripts 
            //       to use to actually insert [NONE:00000000] into list indices.
            //
         }
         self.after_edit();
         return 0;
      }
   }
}
#pragma endregion

#pragma region shout
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
      define_collection_metatable(
         L,
         _wrapper_t::entry_collection_key,
         &wrapper::__gc,
         false,
         &_collections::entries::get_collection_length, // args: wrapper;        return: number
         nullptr,
         &_collections::entries::lookup_item_by_index,  // args: wrapper, index; return: wrapper or nil
         nullptr,
         &_collections::entries::set_item
      );
   }
}
#pragma endregion