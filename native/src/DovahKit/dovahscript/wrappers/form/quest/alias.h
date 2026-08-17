#pragma once
#include "../../base.h"
#include "../../../wrapper.h"
#include "../../../../dovah/forms/components/papyrus.h"

namespace dovah::loaded_forms {
   class Alias;
}

namespace dovahscript::wrappers {
   struct quest_alias : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.quest_alias";
      static constexpr const char*   class_name      = "quest_alias";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Alias;
      static wrapped_type* unwrap(wrapper& w);

      static const char* metatable_key_for(const wrapped_type&);

      static int wrap(lua_State* L, dovah::form_stub* quest, uint32_t aliasID);
      static int wrap(lua_State* L, dovah::form_stub* quest, const wrapped_type* alias);
      static int wrap(lua_State* L, const wrapper& collection, const wrapped_type* alias);
      static int wrap_by_index(lua_State* L, const wrapper& collection, size_t zero_based_index);
   };
   
   struct quest_loc_alias;
   struct quest_ref_alias;
}