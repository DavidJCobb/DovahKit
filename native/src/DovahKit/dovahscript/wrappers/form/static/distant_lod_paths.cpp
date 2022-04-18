#include "distant_lod_paths.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/Static.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::static_distant_lod_paths;
   using wrapped_type = cls::wrapped_type;
}

namespace {
   namespace _getters {
      template<size_t i> int _item(lua_State* L) {
         static_assert(std::tuple_size_v<decltype(wrapped_type::distant_lod_paths)> > i);
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->distant_lod_paths[i].c_str());
         return 1;
      }
   }
   namespace _setters {
      template<size_t i> int _item(lua_State* L) {
         static_assert(std::tuple_size_v<decltype(wrapped_type::distant_lod_paths)> > i);
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         cobb::lua::argcheck(L, lua_isnil(L, 2) || lua_isstring(L, 2), 2, "string or nil expected");
         if (!form)
            return 0;
         self.before_edit();
         if (lua_isnil(L, 2)) {
            form->distant_lod_paths[i].clear();
         } else {
            form->distant_lod_paths[i] = lua_tostring(L, 2);
         }
         self.after_edit();
         return 1;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;

   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "0", &_getters::_item<0> },
      { "1", &_getters::_item<1> },
      { "2", &_getters::_item<2> },
      { "3", &_getters::_item<3> },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "0", &_setters::_item<0> },
      { "1", &_setters::_item<1> },
      { "2", &_setters::_item<2> },
      { "3", &_setters::_item<3> },
   };
}