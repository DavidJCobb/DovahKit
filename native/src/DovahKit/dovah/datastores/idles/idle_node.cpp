#include "./idle_node.h"
#include "./action_node.h"
#include "./passkeys/push_idle_hierarchy_position_to_form.h"
#include "../idles.h"
#include "../../forms/IdleAnimation.h"

namespace dovah::datastores::impl::idles {
   void idle_node::_on_hierarchy_changed(passkeys::push_idle_hierarchy_position_to_form, size_t i) {
      auto loaded = this->stub.load().ptr_cast<loaded_forms::IdleAnimation>();
      if (!loaded)
         return;

      const idle_parent_node* parent_node = this->parent;
      if (i == node::no_index) {
         if (parent_node)
            i = parent_node->index_of_child(*this);
      } else if (!parent_node) {
         i = 0;
      }

      form_stub* parent_stub = nullptr;
      if (parent_node) {
         if (auto* parent_idle = dynamic_cast<const idle_node*>(parent_node)) {
            parent_stub = &parent_idle->stub;
         } else if (auto* parent_action = dynamic_cast<const action_node*>(parent_node)) {
            parent_stub = &parent_action->stub;
         }
      } else {
         i = 0;
      }
         
      if (auto& cb = this->datastore.callbacks.on_any_form_modified.before)
         cb(this->stub);
      //
      if (parent_stub) {
         const auto& graph = loaded->get_behavior_graph_path(false);
         if (parent_stub->form_type == form_type::action) {
            loaded->make_action_root(graph, *parent_stub);
         } else {
            form_stub* previous_stub = nullptr;
            if (i > 0)
               previous_stub = &parent_node->children[i - 1]->stub;
            loaded->make_child(graph, *parent_stub, previous_stub);
         }
      } else {
         loaded->make_loose();
      }
      //
      if (auto& cb = this->datastore.callbacks.on_any_form_modified.after)
         cb(this->stub);
   }
}