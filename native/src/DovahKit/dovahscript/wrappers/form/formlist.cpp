#include "./formlist.h"
#include "dovahscript/wrapper.h"
#include "./formlist/collection_entries.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::formlist;
   using wrapped_type = dovah::loaded_forms::FormList;

   namespace _getters {
      int entries(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return wrapper_likes::native_lists::formlist_entries::push(L, self);
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "entries", &_getters::entries },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = no_functions;

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      wrapper_likes::native_lists::formlist_entries::define_metatable(L);
   }
}