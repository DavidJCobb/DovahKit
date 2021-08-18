#pragma once
#include "../../base.h"
#include "../../../wrapper.h"

namespace dovahscript::wrappers::resource {
   //
   // This wrapper is used for:
   //
   //    dds.images[n]                               | DDSImage[n]
   //    dds.images[n].cubemap_faces.x_pos           | DDSImage[n]/DDSCubX+
   //    dds.images[m].mipmap[n]                     | DDSImage[m]/DDSMipLv[n]
   //    dds.images[m].cubemap_faces.x_pos.mipmap[n] | DDSImage[m]/DDSCubX+/DDSMipLv[n]
   //
   struct dds_image_subresource : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.resource.dds.image";
      static constexpr const char*   class_name      = "dds_image_subresource";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L) noexcept;

      static constexpr const char* cubemap_face_collection_key = "collection<dovah.classes.resource.dds_image_subresource.cubemap_faces>";
      static constexpr const char* mipmap_collection_key       = "collection<dovah.classes.resource.dds_image_subresource.mipmaps>";
   };
}