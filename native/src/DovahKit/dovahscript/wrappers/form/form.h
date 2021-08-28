#pragma once
#include "../../../helpers/eight_cc.h"
#include "../base.h"

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc destruction_root  = "Destruct";
   inline constexpr cobb::eight_cc destruction_stage = "DesStage";
   inline constexpr cobb::eight_cc object_bounds     = "ObBounds";
}

namespace dovahscript::wrappers {
   struct form : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.form";
      static constexpr const char*   class_name      = "form";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;
   };
}