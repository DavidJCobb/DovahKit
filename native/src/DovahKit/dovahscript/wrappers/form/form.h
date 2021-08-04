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
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.form";
      static constexpr const char* class_name     = "form";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;
   };
}