#pragma once
#include "../navmesh.h"

#include "dovah/forms/Navmesh.h"

namespace dovahscript::wrappers {
   struct navmesh_triangle_cover_edge : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.navmesh_triangle_cover_edge";
      static constexpr const char*   class_name      = "triangle_cover_edge";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Navmesh::cover_info;
   };
}