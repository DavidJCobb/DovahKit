#include "./node.h"
#include <cassert>
#include "./passkeys/post_build_edit.h"
#include "./branch_node.h"
#include "../../forms/mixins/StoryManagerNode.h"
#include "../../form_stub.h"

namespace dovah::datastores::impl::story_manager {
   void node::_update_form_hierarchy_data(passkeys::post_build_edit) {
      auto  loaded = this->stub.load();
      auto* casted = dynamic_cast<loaded_forms::mixins::StoryManagerNode*>(&*loaded);
      assert(casted != nullptr);

      form_stub* parent_stub   = nullptr;
      form_stub* previous_stub = nullptr;
      if (this->parent) {
         parent_stub = &this->parent->stub;

         size_t i = this->parent->index_of_child(*this);
         assert(i != branch_node::index_of_none);
         if (i > 0) {
            previous_stub = &this->parent->children[i - 1]->stub;
         }
      }
      casted->parent.set(*loaded, parent_stub);
      casted->previous_sibling.set(*loaded, previous_stub);
      loaded->stub.set_edited(true);
   }
}