#include "root.h"
#include "../../classes.h"
#include "../../util.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"
#include "../../collections.h"

#include "../../../../dovah/forms/Form.h"
#include "script.h"

namespace {
   using namespace editor_script;

   wrappers::papyrus_root::wrapped_t& _unwrap(lua_State* L, wrapper& w) {
      auto* data = wrappers::papyrus_root::unwrap(w);
      if (!data)
         luaL_error(L, "wrapper `%s` has no underlying object", wrappers::papyrus_root::metatable_key);
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
         auto& root = _unwrap(L, self);
         const char* name = lua_tostring(L, 2);
         if (!name)
            return 0;
         auto& list = root.scripts;
         auto  size = list.size();
         for (auto& script : list) {
            if (stricmp(script.name.c_str(), name) == 0) {
               wrapper out = self;
               assert(out.is_collection);
               assert(out.parts[0].signature == cobb::eight_cc("PapyRoot"));
               assert(out.parts[1].signature == cobb::eight_cc("PapyScri"));
               out.into_collection(name);
               return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_script::metatable_key);
            }
         }
         return 0;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto& root = _unwrap(L, self);
         auto  i    = lua_tointeger(L, 2);
         auto& list = root.scripts;
         if (i >= list.size() || i < 0)
            return 0;
         wrapper out = self;
         assert(out.is_collection);
         assert(out.parts[0].signature == cobb::eight_cc("PapyRoot"));
         assert(out.parts[1].signature == cobb::eight_cc("PapyScri"));
         out.into_collection(i);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_script::metatable_key);
      }
      luastackchange_t get_all_item_names(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto& root = _unwrap(L, self);
         auto& list = root.scripts;
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
         out.append_part(cobb::eight_cc("PapyScri"));
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_root::script_collection_key);
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> papyrus_root::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> papyrus_root::metatable_getters = {
      { "parent",  &_getters::parent },
      { "scripts", &_getters::scripts },
   };
   /*static*/ const std::initializer_list<luaL_Reg> papyrus_root::metatable_setters = no_functions;

   /*static*/ void papyrus_root::build_collection_metatables(lua_State* L) {
      define_collection_metatable(
         L,
         wrappers::papyrus_root::script_collection_key,
         &wrapper::__gc,
         true,
         &_collections::scripts::lookup_item_by_name,  // args: wrapper, name;  return: wrapper or nil
         &_collections::scripts::lookup_item_by_index, // args: wrapper, index; return: wrapper or nil
         &_collections::scripts::get_all_item_names    // args: wrapper;        return: table of names
      );
   }

   /*static*/ papyrus_root::wrapped_t* papyrus_root::unwrap(wrapper& w) {
      //
      // TODO: If we decide to use the same metatable for quest alias scripts, then we'll need to 
      // check whether this script data is attached to an alias and if so, return that alias.
      //
      if (w.parts[0].signature != cobb::eight_cc("PapyRoot"))
         return nullptr;
      auto* form = w.get_loaded_form_data<dovah::loaded_forms::Form>();
      if (!form)
         return nullptr;
      return form->get_papyrus_data();
   }
}