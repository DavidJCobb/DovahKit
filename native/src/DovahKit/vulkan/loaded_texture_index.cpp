#include "loaded_texture_index.h"
#include "surface_renderer.h"
#include "scene.h"

namespace vulkanDK {
   void loaded_texture_index::_dec_ref(surface_renderer& sr) {
      if (this->_value < 0)
         return;
      sr.scene.texture_dec_ref(scene_passkey{}, this->_value);
   }
   void loaded_texture_index::_inc_ref(surface_renderer& sr) {
      if (this->_value < 0)
         return;
      ++(this->data(sr).refcount);
   }

   loaded_texture& loaded_texture_index::data(surface_renderer& sr) {
      assert(!this->empty());
      return sr.scene.entities_of_type<loaded_texture>()[this->_value];
   }
   const loaded_texture& loaded_texture_index::data(const surface_renderer& sr) const {
      assert(!this->empty());
      return sr.scene.entities_of_type<loaded_texture>()[this->_value];
   }

   void loaded_texture_index::set(surface_renderer& sr, value_type ti) {
      if (this->_value == ti)
         return;
      _dec_ref(sr);
      this->_value = ti;
      _inc_ref(sr);
   }
}