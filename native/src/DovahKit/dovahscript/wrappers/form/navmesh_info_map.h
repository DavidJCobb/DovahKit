#pragma once
#include "form.h"

namespace dovah::loaded_forms {
   class NavMeshInfoMap;
}

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc navmesh_info_collection    = "NavInfoC";
   inline constexpr cobb::eight_cc navi_precomputed_path      = "NavPPath";
   inline constexpr cobb::eight_cc navi_precomputed_path_node = "NavPthNd";
   inline constexpr cobb::eight_cc navi_road_marker           = "NavRoadM";
   inline constexpr cobb::eight_cc navi_deleted_navmeshes     = "NavDelet";
}

namespace dovahscript::wrappers {
   struct navmesh_info_map : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.navmesh_info_map";
      static constexpr const char*   class_name      = "navmesh_info_map";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::NavMeshInfoMap;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L);
   };
}