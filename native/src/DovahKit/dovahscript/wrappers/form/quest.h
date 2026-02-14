#pragma once
#include "form.h"

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc quest_alias             = "QstAlias"; // only used for the collection
   inline constexpr cobb::eight_cc quest_alias_by_id       = "QstAlsID"; // used for the by-ID collection and for all aliases in both collections
   inline constexpr cobb::eight_cc quest_alias_fill_params = "QstAlsFl";
}

namespace dovahscript::wrappers {
   struct quest : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.quest";
      static constexpr const char*   class_name      = "quest";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L);
   };
}