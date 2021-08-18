#pragma once
#include "../../../wrapper.h"

namespace dovahscript::wrappers::resource {
   struct dds_cubemap_face_list : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.resource.dds.dds_cubemap_face_list";
      static constexpr const char*   class_name      = "dds_cubemap_face_list";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;
   };
}