#include "image.h"
#include "../dds.h"
#include "../raster.h"

#include "../../../systems/editor_script_inner_core.h"
#include "../../../systems/lua_managed_resources.h"
#include "../../../systems/messaging.h"
#include "../../../systems/permissions.h"
#include "../../../systems/userdata.h"

#include "../../../editor_script_core.h"
#include "../../../wrapper_util.h"

#include "../../../cross_thread_tasks/s2m/lambda.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::resource::dds_image_subresource;

   namespace _methods {
      luastackchange_t copy_to_raster(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         size_t  array_index  = 0;
         size_t  mipmap_index = 0;
         uint8_t cubemap_face = 0;
         assert(self.parts[0].signature == wrapper_part_types::resource_dds_image);
         array_index = self.parts[0].index;
         {
            int i = 1;
            switch (self.parts[i].signature) {
               case wrapper_part_types::resource_dds_cubemap_face_x_pos:
                  ++i;
                  cubemap_face = 0;
                  break;
               case wrapper_part_types::resource_dds_cubemap_face_x_neg:
                  ++i;
                  cubemap_face = 1;
                  break;
               case wrapper_part_types::resource_dds_cubemap_face_y_pos:
                  ++i;
                  cubemap_face = 2;
                  break;
               case wrapper_part_types::resource_dds_cubemap_face_y_neg:
                  ++i;
                  cubemap_face = 3;
                  break;
               case wrapper_part_types::resource_dds_cubemap_face_z_pos:
                  ++i;
                  cubemap_face = 4;
                  break;
               case wrapper_part_types::resource_dds_cubemap_face_z_neg:
                  ++i;
                  cubemap_face = 5;
                  break;
            }
            if (self.parts[i].signature == wrapper_part_types::resource_dds_image_mipmap_level) {
               mipmap_index = self.parts[i].index;
            }
         }
         //
         LuaManagedResource* resource = nullptr;
         {
            const auto* base = self.managed_resource;
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [base, array_index, mipmap_index, cubemap_face, &resource]() {
               auto image = base->get_dds_layer(array_index, mipmap_index, cubemap_face);
               if (image.isNull())
                  return;
               resource = DovahKitScriptVMResourceInterface::get().create_resource(image);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
            assert(resource);
         }
         if (!resource)
            return 0;
         return wrappers::resource::raster::wrap_and_push(L, *resource);
      }
   }
   namespace _getters {
   }
   namespace _setters {
   }
}

namespace editor_script::wrappers::resource {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "copy_to_raster", &_methods::copy_to_raster },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };
}