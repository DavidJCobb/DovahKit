#pragma once
#include "form.h"

namespace dovah::loaded_forms {
   class Worldspace;
}

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc worldspace_bounds     = "WrldBnds";
   inline constexpr cobb::eight_cc worldspace_bounds_min = "WrldNAM0";
   inline constexpr cobb::eight_cc worldspace_bounds_max = "WrldNAM9";
   inline constexpr cobb::eight_cc worldspace_large_refs = "WrldLarg"; // TODO: Implement me! // SSE-only
   inline constexpr cobb::eight_cc worldspace_max_height = "WrldMHDt"; // TODO: Implement me!
}

namespace dovahscript::wrappers {
   struct worldspace : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.worldspace";
      static constexpr const char*   class_name      = "worldspace";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Worldspace;
   };
}