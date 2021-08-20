#include "dds.h"
#include "../../core/subsystems/resources.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"

#include "../../tasks/s2m/lambda.h"

#include "dds/collection_images.h"
#include "dds/image.h"
#include "raster.h"

namespace {
   using namespace dovahscript;
   using cls = wrappers::resource::dds;

   namespace _methods {
      int copy_to_raster(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         DovahscriptResource* resource = nullptr;
         {
            const auto* base = self.managed_resource;
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [base, &resource]() {
               QImage layer = base->get_dds_layer(0, 0, 0);
               assert(!layer.isNull());
               resource = core::subsystems::resources::get().create_resource(layer);
            };
            send_script_task(*task);
            delete task;
            assert(resource);
         }
         return push_native_object(resource);
      }
   }
   namespace _getters {
      int images(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::resource_dds_image);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::resource::collections::dds_images_list.registry_key);
      }
      int is_cubemap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         lua_pushboolean(L, self.managed_resource->is_cubemap());
         return 1;
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
      { "images",     &_getters::images },
      { "is_cubemap", &_getters::is_cubemap },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      define_collection_metatable(L, collections::dds_images_list);
   }
}