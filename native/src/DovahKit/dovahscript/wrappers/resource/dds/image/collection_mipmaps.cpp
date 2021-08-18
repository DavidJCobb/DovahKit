#include "collection_mipmaps.h"
#include "../../../../../helpers/lua/error.h"
#include "../../../../core/subsystems/resources/DovahscriptResource.h"
#include "../../../../core/subsystems/userdata.h"
#include "../../../../core/classes.h"
#include "../../../../wrapper.h"

#include "../../dds.h"
#include "../image.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.resource.dds_image_subresource.mipmaps>";
}

namespace {
   static constexpr std::array<cobb::eight_cc, 6> _cubemap_face_parts = {{
      dovahscript::wrapper_part_types::resource_dds_cubemap_face_x_pos,
      dovahscript::wrapper_part_types::resource_dds_cubemap_face_x_neg,
      dovahscript::wrapper_part_types::resource_dds_cubemap_face_y_pos,
      dovahscript::wrapper_part_types::resource_dds_cubemap_face_y_neg,
      dovahscript::wrapper_part_types::resource_dds_cubemap_face_z_pos,
      dovahscript::wrapper_part_types::resource_dds_cubemap_face_z_neg,
   }};
}

namespace {
   using namespace dovahscript;
   
   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr)
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      if (self->managed_resource == nullptr)
         cobb::lua::error(L, "function called with zombie self (expected %s)", collection_metatable_key);
      return *self;
   }
   DovahscriptResource& get_resource(const wrapper& w) {
      return *w.managed_resource;
   }

   int get_collection_length(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto& dds  = get_resource(self);
      lua_pushinteger(L, dds.mipmap_count());
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto& dds  = get_resource(self);
      //
      int isnum;
      int i = lua_tointegerx(L, 2, &isnum);
      if (!isnum)
         return 0;
      if (i <= 0 || i > dds.mipmap_count())
         return 0;
      --i;
      //
      assert(self.is_collection);
      assert(self.parts[0].signature == wrapper_part_types::resource_dds_image);
      assert(self.last_part().signature == wrapper_part_types::resource_dds_image_mipmap_level);
      size_t  array_index  = self.parts[0].index;
      size_t  mipmap_index = i;
      uint8_t cubemap_face = 0;
      bool    is_cubemap   = false;
      for (int j = 0; j < _cubemap_face_parts.size(); ++j) {
         if (self.parts[1].signature == _cubemap_face_parts[j]) {
            cubemap_face = j;
            is_cubemap   = true;
            break;
         }
      }
      //
      wrapper out = self;
      out.into_collection(i);
      return core::subsystems::userdata::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
   }
}

namespace dovahscript::wrappers::resource::collections {
   extern const collection_definition_params dds_image_mipmap_list = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
   };
}