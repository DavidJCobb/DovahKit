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

namespace {
   using namespace editor_script;
   using wrapper_t = wrappers::papyrus_script;
}

#pragma region Collection: "properties"
namespace {
   using namespace editor_script;

   namespace _collections::properties {
      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, wrapper_t::property_collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", wrapper_t::property_collection_key);
         }
         return *self;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self   = get_collection_wrapper(L);
         auto* script = wrapper_t::unwrap(self, false);
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
         auto* script = wrapper_t::unwrap(self, false);
         if (!script)
            luaL_error(L, "wrapper `%s` has no underlying object (deleted?)", wrapper_t::property_collection_key);
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
               out.into_collection(i);
               return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_property::metatable_key);
            }
         }
         return 0;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self   = get_collection_wrapper(L);
         auto* script = wrapper_t::unwrap(self, false);
         if (!script)
            luaL_error(L, "wrapper `%s` has no underlying object (deleted?)", wrapper_t::property_collection_key);
         auto  i    = lua_tointeger(L, 2);
         auto& list = script->properties;
         if (i > list.size() || i <= 0)
            return 0;
         --i;
         wrapper out = self;
         assert(out.is_collection);
         out.into_collection(i);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_property::metatable_key);
      }
      luastackchange_t get_all_item_names(lua_State* L) {
         auto& self   = get_collection_wrapper(L);
         auto* script = wrapper_t::unwrap(self, false);
         if (!script)
            luaL_error(L, "wrapper `%s` has no underlying object (deleted?)", wrapper_t::property_collection_key);
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
         auto& self   = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            luaL_error(L, "script wrapper has no underlying object (deleted?)");
         __assume(script != nullptr);
         //
         lua_pushstring(L, script->name.c_str());
         return 1;
      }
      luastackchange_t properties(lua_State* L) {
         auto& self   = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            luaL_error(L, "script wrapper has no underlying object (deleted?)");
         __assume(script != nullptr);
         //
         wrapper out = self;
         out.append_part(wrapper_part_types::papyrus_property);
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrapper_t::property_collection_key);
      }
   }
   namespace _setters {
      luastackchange_t name(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self   = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            luaL_error(L, "script wrapper has no underlying object (deleted?)");
         __assume(script != nullptr);
         //
         auto* name = lua_tolstring(L, 2, nullptr);
         auto* root = wrappers::papyrus_root::unwrap(self, false);
         assert(root && "How did we manage to access a Papyrus script-object if we didn't manage to access its containing Papyrus root?");
         if (auto* prior = root->lookup_script(name))
            if (prior != script)
               luaL_error(L, "the containing form already has a Papyrus script named \"%s\"", name);
         //
         self.before_edit();
         script->name = name;
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_getters = {
      { "name",       &_getters::name },
      { "properties", &_getters::properties },
   };
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_setters = {
      { "name", &_setters::name },
   };

   /*static*/ wrapper_t::wrapped_t* wrapper_t::unwrap(wrapper& w, bool must_be_end) {
      uint8_t next;
      return papyrus_script::unwrap(w, must_be_end, next);
   }
   /*static*/ wrapper_t::wrapped_t* wrapper_t::unwrap(wrapper& w, bool must_be_end, uint8_t& next_depth) {
      uint8_t next;
      auto*   root = papyrus_root::unwrap(w, false, next);
      next_depth = next;
      if (!root)
         return nullptr;
      if (w.parts[next].signature != wrapper_part_types::papyrus_script)
         return nullptr;
      if (w.is_collection_at_depth(next))
         return nullptr;
      auto& list = root->scripts;
      auto  i    = w.parts[next].index;
      if (i >= list.size())
         return nullptr;
      ++next_depth;
      if (must_be_end && w.depth != next_depth)
         return nullptr;
      return &list[i];
   }

   /*static*/ void wrapper_t::build_collection_metatables(lua_State* L) {
      define_collection_metatable(L, {
         .registry_key           = wrapper_t::property_collection_key,
         .garbage_collection     = &wrapper::__gc,
         //
         .get_all_item_names     = &_collections::properties::get_all_item_names,
         .get_collection_length  = &_collections::properties::get_collection_length,
         .items_are_named        = true,
         .lookup_item_by_name    = &_collections::properties::lookup_item_by_name,
         .lookup_item_by_index   = &_collections::properties::lookup_item_by_index,
      });
   }
}