#pragma once
#include "../base.h"
#include "../../wrapper.h"

namespace dovahscript {
   class DovahscriptResource;
}

namespace dovahscript::wrapper_part_types {
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

namespace dovahscript::wrappers::resource {
   struct dds : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key    = "dovah.classes.resource.dds";
      static constexpr const char*   class_name       = "dds_resource";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L) noexcept;
   };
}