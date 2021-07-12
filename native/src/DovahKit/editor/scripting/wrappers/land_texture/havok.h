#pragma once
#include "../topic_info.h"

#include "../../../../dovah/forms/LandTexture.h"

namespace editor_script::wrappers {
   struct land_texture_havok : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.land_texture_havok";
      static constexpr const char* class_name     = "land_texture_havok";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;
   };
}