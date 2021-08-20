#include "image.h"
#include "../../../core/subsystems/resources.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../push_native_object.h"
#include "../../../send_script_task.h"

#include "../../../tasks/s2m/lambda.h"

#include "image/collection_mipmaps.h"
#include "cubemap_face_list.h"
#include "../dds.h"
#include "../raster.h"

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
   using cls = wrappers::resource::dds_image_subresource;

   namespace _methods {
      int copy_to_raster(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         size_t  array_index  = 0;
         size_t  mipmap_index = 0;
         uint8_t cubemap_face = 0;
         bool    is_cubemap;
         assert(self.parts[0].signature == wrapper_part_types::resource_dds_image);
         array_index = self.parts[0].index;
         {
            int i = 1;
            is_cubemap = false;
            for (int f = 0; f < _cubemap_face_parts.size(); ++f) {
               if (self.parts[i].signature == _cubemap_face_parts[f]) {
                  cubemap_face = f;
                  ++i;
                  is_cubemap = true;
                  break;
               }
            }
            if (self.parts[i].signature == wrapper_part_types::resource_dds_image_mipmap_level) {
               mipmap_index = self.parts[i].index;
            }
         }
         //
         DovahscriptResource* resource = nullptr;
         {
            const auto* base = self.managed_resource;
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [base, array_index, mipmap_index, cubemap_face, is_cubemap, &resource]() {
               if (is_cubemap && !base->is_cubemap())
                  return;
               auto image = base->get_dds_layer(array_index, mipmap_index, cubemap_face);
               if (image.isNull())
                  return;
               resource = core::subsystems::resources::get().create_resource(image);
            };
            send_script_task(*task);
            delete task;
            assert(resource);
         }
         if (!resource)
            return 0;
         return push_native_object(resource);
      }
   }
   namespace _getters {
      int cubemap_faces(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         if (self.depth > 1)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::resource_dds_cubemap);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::resource::dds_cubemap_face_list::metatable_key);
      }
      int mipmaps(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::resource_dds_image_mipmap_level);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, cls::mipmap_collection_key);
      }
   }
   namespace _setters {
   }
}

namespace dovahscript::wrappers::resource {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "copy_to_raster", &_methods::copy_to_raster },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "cubemap_faces", &_getters::cubemap_faces },
      { "mipmaps",       &_getters::mipmaps },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      dovahscript::define_collection_metatable(L, collections::dds_image_mipmap_list);
   }
}