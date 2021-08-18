#include "collection_images.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/resources/DovahscriptResource.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../wrapper.h"

#include "../dds.h"
#include "image.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.resource.dds.images>";
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
      lua_pushinteger(L, dds.texture_array_size());
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
      if (i < 0)
         return 0;
      --i;
      if (i >= dds.texture_array_size())
         return 0;
      wrapper out = self;
      assert(out.is_collection);
      assert(out.parts[0].signature == wrapper_part_types::resource_dds_image);
      out.into_collection(i);
      return core::subsystems::userdata::get().push(L, out, wrappers::resource::dds_image_subresource::metatable_key);
   }
}

namespace dovahscript::wrappers::resource::collections {
   extern const collection_definition_params dds_images_list = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
   };
}