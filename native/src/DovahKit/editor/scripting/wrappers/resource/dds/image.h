#pragma once
#include "../../../wrapper.h"

namespace editor_script::wrapper_part_types {
}

namespace editor_script::wrappers::resource {
   //
   // This wrapper is used for:
   //
   //    dds.images[n]                               | DDSImage[n]
   //    dds.images[n].cubemap_faces.x_pos           | DDSImage[n]/DDSCubX+
   //    dds.images[m].mipmap[n]                     | DDSImage[m]/DDSMipLv[n]
   //    dds.images[m].cubemap_faces.x_pos.mipmap[n] | DDSImage[m]/DDSCubX+/DDSMipLv[n]
   //
   struct dds_image_subresource : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.resource.dds.image";
      static constexpr const char* class_name     = "dds_image_subresource";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* cubemap_face_collection_key = "collection<dovah.classes.resource.dds_image_subresource.cubemap_faces>";
      static constexpr const char* mipmap_collection_key       = "collection<dovah.classes.resource.dds_image_subresource.mipmaps>";
   };
}