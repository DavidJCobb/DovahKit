#include "script.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../core/collections.h"
#include "../../../push_native_object.h"

#include "script/collection_properties.h"

#include "../../../../dovah/form_stub.h"
#include "../../../../dovah/forms/Form.h"

#include "root.h"
#include "property.h"

namespace {
   using namespace dovahscript;
   using wrapper_t = wrappers::papyrus_script;
}

namespace {
   using namespace dovahscript;
   //
   namespace _methods {
      int add_property(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         lua_settop(L, 2);
         //
         auto& self   = get_wrapper_for_thiscall<wrappers::papyrus_script>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            cobb::lua::error(L, "script wrapper has no underlying object (deleted?)");
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "script name (string) expected");
         std::string name = lua_tostring(L, 2);
         if (script->lookup_property(name) != nullptr) {
            cobb::lua::error(L, "script %s already has a property named \"%s\"", script->name.c_str(), name.c_str());
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
         return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_property::metatable_key);
      }
      int remove_all_properties(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrappers::papyrus_script>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            cobb::lua::error(L, "script wrapper has no underlying object (deleted?)");
         //
         self.before_edit();
         core::subsystems::userdata::get().clear_entire_collection(self);
         script->clear_properties(*self.form);
         self.after_edit();
         //
         return 0;
      }
      int remove_property(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self   = get_wrapper_for_thiscall<wrappers::papyrus_script>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            cobb::lua::error(L, "script wrapper has no underlying object (deleted?)");
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
         core::subsystems::userdata::get().remove_from_sequential_collection(*prop); // update sibling wrappers and kill the wrapper
         return 0;
      }
   }
   namespace _getters {
      int name(lua_State* L) {
         auto& self   = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            cobb::lua::error(L, "script wrapper has no underlying object (deleted?)");
         //
         lua_pushstring(L, script->name.c_str());
         return 1;
      }
      int properties(lua_State* L) {
         auto& self   = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            cobb::lua::error(L, "script wrapper has no underlying object (deleted?)");
         //
         wrapper out = self;
         out.append_part(wrapper_part_types::papyrus_property);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::papyrus_property_list.registry_key);
      }
   }
   namespace _setters {
      int name(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self   = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            cobb::lua::error(L, "script wrapper has no underlying object (deleted?)");
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
      int properties(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self   = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* script = wrappers::papyrus_script::unwrap(self, true);
         if (script == nullptr)
            cobb::lua::error(L, "script wrapper has no underlying object (deleted?)");
         //
         if (lua_isnoneornil(L, 2)) { // if the user is assigning nil, just clear all properties
            self.before_edit();
            {
               wrapper temp = self;
               temp.append_part(wrapper_part_types::papyrus_property);
               temp.is_collection = true;
               core::subsystems::userdata::get().clear_entire_collection(temp);
            }
            script->clear_properties(*self.form);
            self.after_edit();
            return 0;
         }
         //
         auto* arg    = (wrapper*) classes::cast_to_class(L, 2, wrappers::collections::papyrus_property_list.registry_key);
         cobb::lua::argcheck(L, arg != nullptr, 2, "expected another Papyrus property collection or nil");
         auto* other  = wrapper_t::unwrap(*arg, false);
         if (other == nullptr)
            cobb::lua::error(L, "script property collection wrapper has no underlying object (deleted?)");
         if (script == other) // self-assignment
            return 0;
         //
         self.before_edit();
         {
            wrapper temp = self;
            temp.append_part(wrapper_part_types::papyrus_property);
            temp.is_collection = true;
            core::subsystems::userdata::get().clear_entire_collection(temp);
         }
         script->clear_properties(*self.form);
         script->clone_properties(*self.form, *other);
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
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

   /*static*/ void wrapper_t::extra_class_setup(lua_State* L) {
      define_collection_metatable(L, collections::papyrus_property_list);
   }
}