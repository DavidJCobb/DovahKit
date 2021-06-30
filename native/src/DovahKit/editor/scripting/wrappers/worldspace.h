#pragma once
#include "form.h"

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc worldspace_bounds     = "WrldBnds";
   inline constexpr cobb::eight_cc worldspace_bounds_min = "WrldNAM0";
   inline constexpr cobb::eight_cc worldspace_bounds_max = "WrldNAM9";
   inline constexpr cobb::eight_cc worldspace_large_refs = "WrldLarg"; // TODO: Implement me! // SSE-only
   inline constexpr cobb::eight_cc worldspace_max_height = "WrldMHDt"; // TODO: Implement me!
}

namespace editor_script::wrappers {
   struct worldspace : public form {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.worldspace";
      static constexpr const char* class_name     = "worldspace";
      static std::initializer_list<luaL_Reg> metatable_methods;
      static std::initializer_list<luaL_Reg> metatable_getters;
      static std::initializer_list<luaL_Reg> metatable_setters;
   };
}