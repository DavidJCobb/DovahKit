#pragma once
#include "helpers/function_pointer.h"

namespace vulkanDK {
   //
   // This struct can be used to monitor `surface_renderer` and be notified of when 
   // certain events take place.
   //
   struct surface_renderer_hooks {

      // Events related to the batch loading or other usage of `rendered_nif` objects.
      struct {
         // Fired as soon as possible after a batch of `rendered_nif` objects have had their 
         // NIF files loaded. This can delay renderer-related processing during a frame draw, 
         // so consider using `on_background_use_complete` instead. The two events will always 
         // fire together, but the timing is better for the latter.
         cobb::function_pointer<void()> on_background_loaded = nullptr;

         // Fired after the renderer has finished "background-using" a batch of `rendered_nif` 
         // objects. "Background-using" here means loading the NIFs' data and then copying that 
         // data into `rendered_mesh` entities.
         //
         // NIFs are background-used during frame draws. This event is sent after the frame's 
         // final queue submission is made, so that your handler won't delay renderer-related 
         // processing.
         cobb::function_pointer<void()> on_background_use_complete = nullptr;
      } nif_batches;

   };
}