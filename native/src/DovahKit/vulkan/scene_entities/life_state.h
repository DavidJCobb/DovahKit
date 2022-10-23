#pragma once

namespace vulkanDK::scene_entities {
   enum class life_state : uint8_t {
      // The scene frame item is "inactive" or "dead."
      empty,

      // The scene frame item should be used when rendering the scene.
      active,

      // The scene frame item ordinarily should be used when rendering the scene, 
      // except that the CPU is loading its resources in the background.
      active_background_loading,

      // The scene frame item ordinarily should be used when rendering the scene, 
      // except that its owned resources have not yet been uploaded to the GPU.
      active_pending_upload,

      // The scene frame item is pending deletion. Its life state will change to 
      // "empty" and its owned GPU-side resources will be wholly deleted once it 
      // is no longer in use by any frame in flight.
      pending_delete,
   };
};