#include "script.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../util.h"
#include "../../../../dovah/form_stub.h"
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
      luastackchange_t set_item(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         constexpr auto index_self  = 1;
         constexpr auto index_key   = 2;
         constexpr auto index_value = 3;
         //
         auto& self   = get_collection_wrapper(L);
         auto* script = wrapper_t::unwrap(self, false);
         if (!script)
            return 0;
         if (!lua_isstring(L, index_key))
            luaL_error(L, "the given key is not auto-convertible to a string and thus cannot be a Papyrus property name");
         std::string name = lua_tostring(L, index_key);
         //
         if (lua_isnoneornil(L, index_value)) {
            //
            // Setting a property to nil should remove it.
            //
            auto& list = script->properties;
            for (auto it = list.begin(); it != list.end(); ++it) {
               auto& prop = *it;
               if (_stricmp(prop.name.c_str(), name.c_str()) == 0) {
                  self.before_edit();
                  prop.clear(*self.form);
                  {
                     wrapper pw = self;
                     assert(pw.is_collection);
                     pw.into_collection(it - list.begin());
                     DovahKitScriptVMUserdataInterface::get().remove_from_sequential_collection(pw);
                  }
                  list.erase(it);
                  self.after_edit();
                  break;
               }
            }
         } else {
            auto* arg = (wrapper*)editor_script::cast_to_class(L, index_value, wrappers::papyrus_property::metatable_key);
            if (!arg)
               luaL_error(L, "you can only overwrite a Papyrus property with nil or with another Papyrus property");
            auto* source = wrappers::papyrus_property::unwrap(*arg, true);
            if (source == nullptr)
               luaL_error(L, "the script property wrapper provided as a value to set has no underlying object (deleted?)");
            __assume(source != nullptr);
            //
            self.before_edit();
            if (auto* prior = script->lookup_property(name)) {
               name = prior->name; // preserve case
               prior->clear(*self.form);
               prior->clone_from(*source, *self.form);
               prior->name = name; // restore name (it may have been changed during the clone operation)
            } else {
               auto& added = script->properties.emplace_back();
               added.clone_from(*source, *self.form);
               added.name = name; // set name after the cloning operation
            }
            self.after_edit();
         }
         return 0;
      }
   }
}
#pragma endregion


namespace {
   using namespace editor_script;
   //
   namespace _methods {
      luastackchange_t add_property(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         lua_settop(L, 2);
         //
         auto& self   = get_wrapper_for_thiscall<wrappers::papyrus_script>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            luaL_error(L, "script wrapper has no underlying object (deleted?)");
         __assume(script != nullptr);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "script name (string) expected");
         std::string name = lua_tostring(L, 2);
         if (script->lookup_property(name) != nullptr) {
            luaL_error(L, "script %s already has a property named \"%s\"", script->name.c_str(), name.c_str());
         }
         self.before_edit();
         auto& s  = script->properties.emplace_back();
         s.name   = name;
         s.type   = dovah::loaded_forms::components::papyrus::property_type::integer;
         s.status = dovah::loaded_forms::components::papyrus::script_data::property_status::altered;
         s.values.emplace_back(0);
         self.after_edit();
         //
         wrapper out = self;
         out.append_part(wrapper_part_types::papyrus_property);
         out.is_collection = true;
         out.into_collection(script->properties.size() - 1);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_property::metatable_key);
      }
      luastackchange_t remove_all_properties(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrappers::papyrus_script>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            luaL_error(L, "script wrapper has no underlying object (deleted?)");
         //
         self.before_edit();
         DovahKitScriptVMUserdataInterface::get().clear_entire_collection(self);
         script->clear_properties(*self.form);
         self.after_edit();
         //
         return 0;
      }
      luastackchange_t remove_property(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self   = get_wrapper_for_thiscall<wrappers::papyrus_script>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            luaL_error(L, "script wrapper has no underlying object (deleted?)");
         //
         // The way this works is fairly simple. The first non-self argument can be a wrapped 
         // collection item, the name of a collection item, or the index of a collection item. 
         // How do we make use of this? Well, we need to remove the desired item from the 
         // wrapped collection, and then we need to tell the VM to update all sibling wrappers. 
         // Consider this collection:
         //
         //    A B C D E
         //
         // If the script sets a variable to the fourth collection item, then that variable 
         // will point at "D." If the script then removes the second collection item, we want 
         // to make sure that the script variable that points at "D" still does point at "D," 
         // despite "D" no longer being the fourth item but rather now being the third item. 
         // This means that when we delete items from wrapped sequential collections, we need 
         // to also update the wrappers for all items in the collection that came after the 
         // deleted item.
         //
         lua_settop(L, 2); // remove extra arguments
         auto* prop = wrapper_from_stack<wrappers::papyrus_property>(L, 2);
         if (!prop) {
            //
            // We weren't given a wrapped property, so what we received was either an index, 
            // a name, or an invalid argument. Pass it directly to the collection; that's the 
            // easiest way to "convert it to a wrapper" while avoiding code duplication. 
            // Essentially,
            //
            //    arg = self.scripts[arg]
            //
            lua_getfield(L, 1, "properties"); // STACK: - [ self, arg, self.scripts ] +
            lua_rotate  (L, 2, 1);            // STACK: - [ self, self.scripts, arg ] +
            lua_gettable(L, 2);               // STACKL - [ self, self.scripts, self.scripts[arg] ] +
            if (lua_isnoneornil(L, 3))
               return 0;
            prop = wrapper_from_stack<wrappers::papyrus_property>(L, 3);
            if (!prop)
               return 0;
         }
         if (!prop->depth) // Didn't manage to build a usable wrapper for the search-and-remove. Exit early.
            return 0;
         auto index = prop->last_part().index;
         //
         self.before_edit();
         script->remove_property(*self.form, index); // remove the underlying wrapped object
         self.after_edit();
         //
         DovahKitScriptVMUserdataInterface::get().remove_from_sequential_collection(*prop); // update sibling wrappers and kill the wrapper
         return 0;
      }
   }
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
      luastackchange_t properties(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self   = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            luaL_error(L, "script wrapper has no underlying object (deleted?)");
         __assume(script != nullptr);
         //
         if (lua_isnoneornil(L, 2)) { // if the user is assigning nil, just clear all properties
            self.before_edit();
            {
               wrapper temp = self;
               temp.append_part(wrapper_part_types::papyrus_property);
               temp.is_collection = true;
               DovahKitScriptVMUserdataInterface::get().clear_entire_collection(temp);
            }
            script->clear_properties(*self.form);
            self.after_edit();
            return 0;
         }
         //
         auto* arg    = (wrapper*) editor_script::cast_to_class(L, 2, wrapper_t::property_collection_key);
         luaL_argcheck(L, arg != nullptr, 2, "expected another Papyrus property collection or nil");
         __assume(arg != nullptr);
         auto* other  = wrapper_t::unwrap(*arg, false);
         if (other == nullptr)
            luaL_error(L, "script property collection wrapper has no underlying object (deleted?)");
         if (script == other) // self-assignment
            return 0;
         __assume(other != nullptr);
         //
         self.before_edit();
         {
            wrapper temp = self;
            temp.append_part(wrapper_part_types::papyrus_property);
            temp.is_collection = true;
            DovahKitScriptVMUserdataInterface::get().clear_entire_collection(temp);
         }
         script->clear_properties(*self.form);
         script->clone_properties(*self.form, *other);
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_methods = {
      { "add_property",          &_methods::add_property },
      { "remove_all_properties", &_methods::remove_all_properties },
      { "remove_property",       &_methods::remove_property },
   };
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_getters = {
      { "name",       &_getters::name },
      { "properties", &_getters::properties },
   };
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_setters = {
      { "name",       &_setters::name },
      { "properties", &_setters::properties },
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
         .items_are_named        = true,
         .lookup_item_by_name    = &_collections::properties::lookup_item_by_name,
         .set_item               = &_collections::properties::set_item,
      });
   }
}