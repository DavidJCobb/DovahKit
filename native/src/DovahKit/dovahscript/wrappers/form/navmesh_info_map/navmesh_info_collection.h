#pragma once
#include "../navmesh_info_map.h"

#include "dovah/forms/NavMeshInfoMap.h"

namespace dovahscript::wrappers {
   struct navmesh_info_collection : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.navmesh_info_collection";
      static constexpr const char*   class_name      = "navmesh_info_collection";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::NavMeshInfoMap::navmesh_info_collection;
   };
}