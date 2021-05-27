#pragma once
#include "../../wrapper.h"

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc resource_dds_image              = "DDSImage";
   inline constexpr cobb::eight_cc resource_dds_cubemap            = "DDSCubRt";
   inline constexpr cobb::eight_cc resource_dds_cubemap_face_x_pos = "DDSCubX+";
   inline constexpr cobb::eight_cc resource_dds_cubemap_face_x_neg = "DDSCubX-";
   inline constexpr cobb::eight_cc resource_dds_cubemap_face_y_pos = "DDSCubY+";
   inline constexpr cobb::eight_cc resource_dds_cubemap_face_y_neg = "DDSCubY-";
   inline constexpr cobb::eight_cc resource_dds_cubemap_face_z_pos = "DDSCubZ+";
   inline constexpr cobb::eight_cc resource_dds_cubemap_face_z_neg = "DDSCubZ-";
   inline constexpr cobb::eight_cc resource_dds_image_mipmap_level = "DDSMipLv";
   //
   // dds.images[n]                               | DDSImage[n]
   // dds.images[n].cubemap_faces                 | DDSImage[n]/DDSCubRt
   // dds.images[n].cubemap_faces.x_pos           | DDSImage[n]/DDSCubX+
   // dds.images[m].mipmap[n]                     | DDSImage[m]/DDSMipLv[n]
   // dds.images[m].cubemap_faces.x_pos.mipmap[n] | DDSImage[m]/DDSCubX+/DDSMipLv[n]
   //
}

namespace editor_script::wrappers::resource {
   struct dds : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.resource.dds";
      static constexpr const char* class_name     = "dds_resource";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static int wrap_and_push(lua_State*, LuaManagedResource&);
   };
}