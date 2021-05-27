#pragma once
#include "../../../wrapper.h"

namespace editor_script::wrappers::resource {
   struct dds_cubemap_face_list : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.resource.dds.dds_cubemap_face_list";
      static constexpr const char* class_name     = "dds_cubemap_face_list";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;
   };
}