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
   static constexpr std::array<cobb::eight_cc, 6> _cubemap_face_parts = {{
      editor_script::wrapper_part_types::resource_dds_cubemap_face_x_pos,
      editor_script::wrapper_part_types::resource_dds_cubemap_face_x_neg,
      editor_script::wrapper_part_types::resource_dds_cubemap_face_y_pos,
      editor_script::wrapper_part_types::resource_dds_cubemap_face_y_neg,
      editor_script::wrapper_part_types::resource_dds_cubemap_face_z_pos,
      editor_script::wrapper_part_types::resource_dds_cubemap_face_z_neg,
   }};
}

#pragma region Collection: "mipmaps"
namespace {
   using namespace editor_script;

   namespace _collections::mipmaps {
      using cls        = wrappers::resource::dds_image_subresource;
      using model_t    = ObservableStandardItemModel;
      using observer_t = ObservableStandardItemModelObserver;

      static constexpr auto collection_key = cls::mipmap_collection_key;

      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", collection_key);
         }
         __assume(self != nullptr);
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
         lua_pushinteger(L, dds.mipmap_count());
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
         return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::metatable_key);
      }
   }
}
#pragma endregion

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
         LuaManagedResource* resource = nullptr;
         {
            const auto* base = self.managed_resource;
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [base, array_index, mipmap_index, cubemap_face, is_cubemap, &resource]() {
               if (is_cubemap && !base->is_cubemap())
                  return;
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
      luastackchange_t mipmaps(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.managed_resource)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::resource_dds_image_mipmap_level);
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::mipmap_collection_key);
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
      { "mipmaps", &_getters::mipmaps },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };
}