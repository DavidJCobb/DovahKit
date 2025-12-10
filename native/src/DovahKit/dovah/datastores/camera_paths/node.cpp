#include "./node.h"
#include <cassert>
#include "helpers/vectors/move_item_to_index.h"
#include "./passkeys/initial_build.h"
#include "./passkeys/post_build_edit.h"
#include "../camera_paths.h"

#include "../../files/file_load_order.h"
#include "../../forms/CameraPath.h"
#include "../../form_stub.h"

namespace dovah::datastores::impl::camera_paths {
   bool node::is_defined_in_non_active_file() const noexcept {
      return !this->stub.get_owning_load_order().is_defined_in_active_file(this->stub);
   }

   // initial build:
   node::internal_sort_state& node::_get_sort_state(passkeys::initial_build) noexcept {
      return this->_sort_state;
   }

   // post-build:
   void node::_update_form_hierarchy_data(passkeys::post_build_edit) {
      auto loaded = this->stub.load().ptr_cast<loaded_forms::CameraPath>();
      if (!loaded)
         return;

      form_stub* parent_stub   = nullptr;
      form_stub* previous_stub = nullptr;
      if (this->parent) {
         if (auto* casted = this->parent_as_node()) {
            parent_stub = &casted->stub;
         }
         size_t i = this->parent->index_of_child(*this);
         if (i > 0)
            previous_stub = &this->parent->children[i - 1]->stub;
      }
      loaded->parent.set(*loaded, parent_stub);
      loaded->previous_sibling.set(*loaded, previous_stub);
   }
}