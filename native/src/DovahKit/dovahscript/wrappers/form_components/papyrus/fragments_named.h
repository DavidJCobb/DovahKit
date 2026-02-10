#pragma once
#include "../papyrus.h"

namespace dovahscript::wrappers {
   struct papyrus_fragments_named : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.papyrus_fragments_named";
      static constexpr const char*   class_name      = "papyrus_fragments_named";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;
   };
}