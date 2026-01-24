#pragma once
#include "../navmesh.h"

#include "dovah/forms/Navmesh.h"

namespace dovahscript::wrappers {
   struct navmesh_edge_link : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.navmesh_edge_link";
      static constexpr const char*   class_name      = "edge_link";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Navmesh::edge_link;
   };
}