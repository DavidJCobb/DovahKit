#include "script.h"
#include "../../classes.h"
#include "../../util.h"
#include "../../../../dovah/form_stub.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"
#include "../../collections.h"

#include "../../../../dovah/forms/Form.h"
#include "root.h"
#include "property.h"

#pragma region Collection: "properties"
namespace {
   using namespace editor_script;

   namespace _collections::properties {
      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, wrappers::papyrus_script::property_collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", wrappers::papyrus_script::property_collection_key);
         }
         return *self;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self   = get_collection_wrapper(L);
         auto* script = wrappers::papyrus_script::unwrap(self);
         if (!script) {
            lua_pushinteger(L, 0);
            return 1;
         }
         lua_pushinteger(L, script->properties.size());
         return 1;
      }
      luastackchange_t lookup_item_by_name(lua_State* L) {
         //
         // args: wrapper<papyrus_root>, name
         //
         auto& self   = get_collection_wrapper(L);
         auto* script = wrappers::papyrus_script::unwrap(self);
         if (!script)
            luaL_error(L, "wrapper `%s` has no underlying object (deleted?)", wrappers::papyrus_script::property_collection_key);
         const char* name = lua_tostring(L, 2);
         if (!name)
            return 0;
         auto& list = script->properties;
         auto  size = list.size();
         for (size_t i = 0; i < size; ++i) {
            auto& prop = list[i];
            if (stricmp(prop.name.c_str(), name) == 0) {
               wrapper out = self;
               assert(out.is_collection);
               assert(out.parts[0].signature == cobb::eight_cc("PapyRoot"));
               assert(out.parts[1].signature == cobb::eight_cc("PapyScri"));
               assert(out.parts[2].signature == cobb::eight_cc("PapyProp"));
               out.into_collection(i);
               return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_property::metatable_key);
            }
         }
         return 0;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self   = get_collection_wrapper(L);
         auto* script = wrappers::papyrus_script::unwrap(self);
         if (!script)
            luaL_error(L, "wrapper `%s` has no underlying object (deleted?)", wrappers::papyrus_script::property_collection_key);
         auto  i    = lua_tointeger(L, 2);
         auto& list = script->properties;
         if (i > list.size() || i <= 0)
            return 0;
         --i;
         wrapper out = self;
         assert(out.is_collection);
         assert(out.parts[0].signature == cobb::eight_cc("PapyRoot"));
         assert(out.parts[1].signature == cobb::eight_cc("PapyScri"));
         assert(out.parts[2].signature == cobb::eight_cc("PapyProp"));
         out.into_collection(i);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_property::metatable_key);
      }
      luastackchange_t get_all_item_names(lua_State* L) {
         auto& self   = get_collection_wrapper(L);
         auto* script = wrappers::papyrus_script::unwrap(self);
         if (!script)
            luaL_error(L, "wrapper `%s` has no underlying object (deleted?)", wrappers::papyrus_script::property_collection_key);
         auto& list = script->properties;
         //
         lua_createtable(L, 0, list.size());
         auto index_tbl = lua_gettop(L);
         //
         for (auto& prop : list) {
            lua_pushboolean(L, true);
            lua_setfield(L, index_tbl, prop.name.c_str());
         }
         return 1;
      }
   }
}
#pragma endregion


namespace {
   using namespace editor_script;
   //
   namespace _getters {
      luastackchange_t name(lua_State* L) {
         auto& self   = get_wrapper_for_thiscall<wrappers::papyrus_script>(L);
         auto* script = wrappers::papyrus_script::unwrap(self);
         if (script == nullptr)
            luaL_error(L, "script wrapper has no underlying object (deleted?)");
         __assume(script != nullptr);
         //
         lua_pushstring(L, script->name.c_str());
         return 1;
      }
      luastackchange_t properties(lua_State* L) {
         auto& self   = get_wrapper_for_thiscall<wrappers::papyrus_script>(L);
         auto* script = wrappers::papyrus_script::unwrap(self);
         if (script == nullptr)
            luaL_error(L, "script wrapper has no underlying object (deleted?)");
         __assume(script != nullptr);
         //
         wrapper out = self;
         out.append_part(cobb::eight_cc("PapyProp"));
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_script::property_collection_key);
      }
   }
   namespace _setters {
      luastackchange_t name(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self   = get_wrapper_for_thiscall<wrappers::papyrus_script>(L);
         auto* script = wrappers::papyrus_script::unwrap(self);
         if (script == nullptr)
            luaL_error(L, "script wrapper has no underlying object (deleted?)");
         __assume(script != nullptr);
         //
         self.before_edit();
         script->name = lua_tolstring(L, 2, nullptr);
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> papyrus_script::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> papyrus_script::metatable_getters = {
      { "name",       &_getters::name },
      { "properties", &_getters::properties },
   };
   /*static*/ const std::initializer_list<luaL_Reg> papyrus_script::metatable_setters = {
      { "name", &_setters::name },
   };

   /*static*/ papyrus_script::wrapped_t* papyrus_script::unwrap(wrapper& w) {
      if (w.parts[1].signature != cobb::eight_cc("PapyScri"))
         return nullptr;
      if (w.depth == 2) // if w.parts[1] is the last part
         if (w.is_collection)
            return nullptr;
      auto* root = papyrus_root::unwrap(w);
      if (!root)
         return nullptr;
      auto& list = root->scripts;
      auto  i    = w.parts[1].index;
      if (i >= list.size())
         return nullptr;
      return &list[i];
   }

   /*static*/ void papyrus_script::build_collection_metatables(lua_State* L) {
      define_collection_metatable(
         L,
         papyrus_script::property_collection_key,
         &wrapper::__gc,
         true,
         &_collections::properties::get_collection_length, // args: wrapper;        return: number
         &_collections::properties::lookup_item_by_name,
         &_collections::properties::lookup_item_by_index,  // args: wrapper, index; return: wrapper or nil
         &_collections::properties::get_all_item_names
      );
   }
}