#pragma once
#include <cstdint>
#include "helpers/passkey.h"

namespace vulkanDK {
   class loaded_texture;
   class scene;
   class surface_renderer;

   //
   // Loaded textures are refcounted, and will be unloaded when they're unused 
   // (except for specific cases in which they are persistent, e.g. the default 
   // land textures for landscapes). This means that when other scene entities 
   // refer to a loaded texture, these references must maintain the texture's 
   // refcount.
   // 
   // The naive approach would be to use a scene entity handle. However, this 
   // introduces two problems:
   // 
   //    a) Handles are meant for use by systems outside of the renderer, so 
   //       creating a handle type means either exposing loaded textures to 
   //       the outside world (no use case), or doing extra work to prevent 
   //       their exposure.
   // 
   //    b) Handles consist of a pointer to the surface renderer and the index 
   //       of a scene entity. Scene entities are stored inside of the surface 
   //       renderer's scene, however, and are managed top-down by the surface 
   //       renderer. This means that the surface renderer would end up storing 
   //       and following a bunch of pointers to itself. It's a waste of space 
   //       and processing power.
   // 
   // For these reasons, it's better to have scene entities refer to loaded 
   // textures by the textures' indices in the loaded texture entity list. 
   // However, we still benefit from wrapping the indices in a helper struct 
   // to ensure that we don't forget to maintain the refcount.
   //
   struct loaded_texture_index {
      public:
         using value_type = int32_t;   

         static constexpr const value_type none = -1;

      protected:
         using scene_passkey = cobb::passkey<loaded_texture_index, scene>;

         value_type _value = none;

         void _dec_ref(surface_renderer&);
         void _inc_ref(surface_renderer&);

      public:
         constexpr bool empty() const noexcept { return this->_value < 0; }

         loaded_texture& data(surface_renderer&);
         const loaded_texture& data(const surface_renderer&) const;
         constexpr value_type value() const noexcept { return this->_value; }
         
         inline void clear(surface_renderer& sr) {
            this->set(sr, none);
         }
         void set(surface_renderer&, value_type);

         constexpr operator value_type() const noexcept { return value(); }
   };
}