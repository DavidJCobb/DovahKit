#pragma once

namespace vulkanDK::scene_entities {
   enum class life_state {
      // The scene frame item is "inactive" or "dead."
      empty,

      // The scene frame item should be used when rendering the scene.
      active,

      // The scene frame item ordinarily should be used when rendering the scene, 
      // except that its owned resources have not yet been uploaded to the GPU.
      active_pending_upload,

      // The scene frame item is pending deletion. Its life state will change to 
      // "empty" and its owned GPU-side resources will be wholly deleted once it 
      // is no longer in use by any frame in flight.
      pending_delete,

      // The scene frame item was pending deletion, but has been recycled and 
      // made active. The owned GPU-side resources that were pending deletion are 
      // still pending deletion; when they are finally deleted, its life state 
      // will change to "active."
      active_recycle,
   };
};