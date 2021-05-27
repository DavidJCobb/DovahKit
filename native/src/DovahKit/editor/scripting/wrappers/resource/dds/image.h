#pragma once
#include "../../../wrapper.h"

namespace editor_script::wrapper_part_types {
}

namespace editor_script::wrappers::resource {
   struct dds_image_subresource : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.resource.dds.image";
      static constexpr const char* class_name     = "dds_image_subresource";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;
   };
}