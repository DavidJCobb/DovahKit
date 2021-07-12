#pragma once
#include "form.h"

#include "../../../dovah/forms/LandTexture.h"

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc land_texture_grass   = "LTexGras";
   inline constexpr cobb::eight_cc land_texture_physics = "LTexPhys";
}

namespace editor_script::wrappers {
   struct land_texture : public form {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.land_texture";
      static constexpr const char* class_name     = "land_texture";
      static std::initializer_list<luaL_Reg> metatable_methods;
      static std::initializer_list<luaL_Reg> metatable_getters;
      static std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* grasses_collection_key = "collection<dovah.classes.land_texture.grasses>";
      static void build_collection_metatables(lua_State* L);
   };
}