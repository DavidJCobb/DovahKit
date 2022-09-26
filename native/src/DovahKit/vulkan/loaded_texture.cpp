#include "loaded_texture.h"

namespace vulkanDK {
   void loaded_texture::mark_for_delete() {
      base::_mark_for_delete<loaded_texture>();
      if (this->owned_gpu_resources.has_current()) {
         //
         // A pending-delete entity will have both "current" and "outdated" resources 
         // if it was marked for delete after recycling began, but before recycling 
         // could complete. Any such entity should be considered irrecoverable, so 
         // let's clear out our texture path and other data so that we get skipped 
         // more quickly when attempts are made to look up and reuse a loaded texture 
         // by file path (`scene::reuse_scene_texture`).
         //
         this->w = 0;
         this->h = 0;
         this->path.clear();
      }
   }
   void loaded_texture::reset() {
      base::_reset<loaded_texture>();
      this->w = 0;
      this->h = 0;
      this->path.clear();
   }

   bool loaded_texture::persist_for_life_of_renderer() const {
      return (this->flags & (flag::is_default_land_texture)) != 0;
   }
}