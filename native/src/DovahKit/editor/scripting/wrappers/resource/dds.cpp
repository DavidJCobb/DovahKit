#include "dds.h"
#include "raster.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/lua_managed_resources.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"

#include "dds/image.h"

#pragma region Collection: "images"
namespace {
   using namespace editor_script;

   namespace _collections::images {
      using cls        = wrappers::resource::dds;
      using model_t    = ObservableStandardItemModel;
      using observer_t = ObservableStandardItemModelObserver;

      static constexpr auto collection_key = cls::image_collection_key;

      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", collection_key);
         }
         if (self->managed_resource == nullptr) {
            luaL_error(L, "function called with zombie self (expected %s)", collection_key);
         }
         return *self;
      }
      LuaManagedResource& get_resource(const wrapper& w) {
         return *w.managed_resource;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto& dds  = get_resource(self);
         lua_pushinteger(L, dds.texture_array_size());
         return 1;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto& dds  = get_resource(self);
         //
         int isnum;
         int i = lua_tointegerx(L, 2, &isnum) - 1;
         if (!isnum)
            return 0;
         if (i <= 0)
            return 0;
         --i;
         if (i >= dds.texture_array_size())
            return 0;
         wrapper out = self;
         assert(out.is_collection);
         assert(out.parts[0].signature == wrapper_part_types::resource_dds_image);
         out.into_collection(i);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
      }
   }
}
#pragma endregion

namespace {
   using namespace editor_script;
   using cls = wrappers::resource::dds;

   namespace _methods {
      luastackchange_t copy_to_raster(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         LuaManagedResource* resource = nullptr;
         {
            const auto* base = self.managed_resource;
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [base, &resource]() {
               resource = DovahKitScriptVMResourceInterface::get().create_resource(base->get_dds_layer(0, 0, 0));
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
            assert(resource);
         }
         return wrappers::resource::raster::wrap_and_push(L, *resource);
      }
   }
   namespace _getters {
      luastackchange_t images(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::resource_dds_image);
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::image_collection_key);
      }
      luastackchange_t is_cubemap(lua_State* L) {
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

namespace editor_script::wrappers::resource {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "copy_to_raster", &_methods::copy_to_raster },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "images",     &_getters::images },
      { "is_cubemap", &_getters::is_cubemap },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };

   /*static*/ int cls::wrap_and_push(lua_State* L, LuaManagedResource& resource) {
      assert(resource.resource_type() == lua_managed_resource_type::raster);
      wrapper out;
      out.type = wrapper_type::lua_managed_resource;
      out.managed_resource = &resource;
      return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::metatable_key);
   }
}