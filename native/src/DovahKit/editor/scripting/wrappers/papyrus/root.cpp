#include "root.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../util.h"
#include "../../wrapper_util.h"
#include "../../collections.h"

#include "../../../../dovah/forms/Form.h"
#include "../quest/alias.h"
#include "script.h"

namespace {
   using namespace editor_script;
   using wrapper_t = wrappers::papyrus_root;

   wrappers::papyrus_root::wrapped_t& _unwrap(lua_State* L, wrapper& w) {
      auto* data = wrappers::papyrus_root::unwrap(w, true);
      if (!data)
         luaL_error(L, "wrapper `%s` has no underlying object (deleted?)", wrappers::papyrus_root::metatable_key);
      __assume(data != nullptr);
      return *data;
   }
}

#pragma region Collection: "scripts"
namespace {
   using namespace editor_script;

   namespace _collections::scripts {
      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, wrappers::papyrus_root::script_collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", wrappers::papyrus_root::script_collection_key);
         }
         return *self;
      }

      luastackchange_t lookup_item_by_name(lua_State* L) {
         //
         // args: wrapper<papyrus_root>, name
         //
         auto& self = get_collection_wrapper(L);
         auto* root = wrapper_t::unwrap(self, false);
         if (!root)
            luaL_error(L, "wrapper `%s` has no underlying object (deleted?)", wrapper_t::script_collection_key);
         const char* name = lua_tostring(L, 2);
         if (!name)
            return 0;
         auto& list = root->scripts;
         auto  size = list.size();
         for (size_t i = 0; i < size; ++i) {
            auto& script = list[i];
            if (stricmp(script.name.c_str(), name) == 0) {
               wrapper out = self;
               assert(out.is_collection);
               out.into_collection(i);
               return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_script::metatable_key);
            }
         }
         return 0;
      }
      luastackchange_t get_all_item_names(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* root = wrapper_t::unwrap(self, false);
         if (!root)
            luaL_error(L, "wrapper `%s` has no underlying object (deleted?)", wrapper_t::script_collection_key);
         auto& list = root->scripts;
         //
         lua_createtable(L, 0, list.size());
         auto index_tbl = lua_gettop(L);
         //
         for (auto& script : list) {
            lua_pushboolean(L, true);
            lua_setfield(L, index_tbl, script.name.c_str());
         }
         return 1;
      }
   }
}
#pragma endregion

namespace {
   using namespace editor_script;
   //
   namespace _methods {
      luastackchange_t add_script(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         lua_settop(L, 2);
         //
         auto& self = get_wrapper_for_thiscall<wrappers::papyrus_root>(L);
         auto& root = _unwrap(L, self);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "script name (string) expected");
         std::string script_name = lua_tostring(L, 2);
         if (root.lookup_script(script_name) != nullptr) {
            // TODO: vary text for scripts on aliases
            luaL_argcheck(L, false, 2, "a script with this name is already present");
         }
         self.before_edit();
         auto& s = root.scripts.emplace_back();
         s.name = script_name;
         self.after_edit();
         //
         wrapper out = self;
         out.append_part(wrapper_part_types::papyrus_script);
         out.is_collection = true;
         out.into_collection(root.scripts.size() - 1);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_script::metatable_key);
      }
      luastackchange_t remove_script(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrappers::papyrus_root>(L);
         auto& root = _unwrap(L, self);
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
         auto* script = wrapper_from_stack<wrappers::papyrus_script>(L, 2);
         if (!script) {
            //
            // We weren't given a wrapped script, so what we received was either an index, a 
            // name, or an invalid argument. Pass it directly to the collection; that's the 
            // easiest way to "convert it to a wrapper" while avoiding code duplication. 
            // Essentially,
            //
            //    arg = self.scripts[arg]
            //
            lua_getfield(L, 1, "scripts"); // STACK: - [ self, arg, self.scripts ] +
            lua_rotate  (L, 2, 1);         // STACK: - [ self, self.scripts, arg ] +
            lua_gettable(L, 2);            // STACKL - [ self, self.scripts, self.scripts[arg] ] +
            if (lua_isnoneornil(L, 3))
               return 0;
            script = wrapper_from_stack<wrappers::papyrus_script>(L, 3);
            if (!script)
               return 0;
         }
         if (!script->depth) // Didn't manage to build a usable wrapper for the search-and-remove. Exit early.
            return 0;
         auto index = script->last_part().index;
         //
         self.before_edit();
         root.remove_script(*self.form, index); // remove the underlying wrapped object
         self.after_edit();
         //
         DovahKitScriptVMUserdataInterface::get().remove_from_sequential_collection(*script); // update sibling wrappers and kill the wrapper
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t parent(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::papyrus_root>(L);
         if (!self.stub)
            return 0;
         //
         // TODO: If we decide to use the same metatable for quest alias scripts, then we'll need to 
         // check whether this script data is attached to an alias and if so, return that alias.
         //
         wrapper out;
         auto* mt = wrap_form(out, self.stub);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t scripts(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::papyrus_root>(L);
         auto& root = _unwrap(L, self);
         wrapper out = self;
         out.append_part(wrapper_part_types::papyrus_script);
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_root::script_collection_key);
      }
   }
   namespace _setters {
      luastackchange_t scripts(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto& root = _unwrap(L, self);
         //
         if (lua_isnoneornil(L, 2)) { // if the user is assigning nil, just clear all scripts
            self.before_edit();
            {
               wrapper temp = self;
               temp.append_part(wrapper_part_types::papyrus_script);
               temp.is_collection = true;
               DovahKitScriptVMUserdataInterface::get().clear_entire_collection(temp);
            }
            for (auto& s : root.scripts)
               s.clear(*self.form);
            root.scripts.clear();
            self.after_edit();
            return 0;
         }
         //
         auto* arg   = (wrapper*) editor_script::cast_to_class(L, 2, wrapper_t::script_collection_key);
         luaL_argcheck(L, arg != nullptr, 2, "expected another Papyrus script collection or nil");
         auto& other = _unwrap(L, *arg);
         if (&root == &other) // self-assignment
            return 0;
         //
         self.before_edit();
         {
            wrapper temp = self;
            temp.append_part(wrapper_part_types::papyrus_script);
            temp.is_collection = true;
            DovahKitScriptVMUserdataInterface::get().clear_entire_collection(temp);
         }
         for (auto& s : root.scripts)
            s.clear(*self.form);
         size_t size = other.scripts.size();
         root.scripts.clear();
         root.scripts.resize(size);
         for (size_t i = 0; i < size; ++i) {
            root.scripts[i].clone_from(other.scripts[i], *self.form);
         }
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> papyrus_root::metatable_methods = {
      { "add_script",    &_methods::add_script },
      { "remove_script", &_methods::remove_script },
   };
   /*static*/ const std::initializer_list<luaL_Reg> papyrus_root::metatable_getters = {
      { "parent",  &_getters::parent },
      { "scripts", &_getters::scripts },
   };
   /*static*/ const std::initializer_list<luaL_Reg> papyrus_root::metatable_setters = {
      { "scripts", &_setters::scripts },
   };

   /*static*/ void papyrus_root::build_collection_metatables(lua_State* L) {
      define_collection_metatable(L, {
         .registry_key          = wrapper_t::script_collection_key,
         .garbage_collection    = &wrapper::__gc,
         //
         .get_all_item_names     = &_collections::scripts::get_all_item_names,
         .items_are_named        = true,
         .lookup_item_by_name    = &_collections::scripts::lookup_item_by_name,
      });
   }

   /*static*/ papyrus_root::wrapped_t* papyrus_root::unwrap(wrapper& w, bool must_be_end) {
      uint8_t dummy;
      return papyrus_root::unwrap(w, must_be_end, dummy);
   }
   /*static*/ papyrus_root::wrapped_t* papyrus_root::unwrap(wrapper& w, bool must_be_end, uint8_t& next_depth) {
      next_depth = 0;
      if (auto* alias = quest_alias::unwrap(w)) {
         if (w.parts[1].signature != wrapper_part_types::papyrus_root)
            return nullptr;
         next_depth = 2;
         if (must_be_end && w.depth != next_depth)
            return nullptr;
         return &alias->script_data;
      }
      if (w.parts[0].signature != wrapper_part_types::papyrus_root)
         return nullptr;
      auto* form = w.get_loaded_form_data<dovah::loaded_forms::Form>();
      if (!form)
         return nullptr;
      next_depth = 1;
      if (must_be_end && w.depth != next_depth)
         return nullptr;
      return form->get_papyrus_data();
   }
}