#include "./idle_node.h"
#include "./action_node.h"
#include "./passkeys/action_update_root.h"
#include "./passkeys/check_is_clearing.h"
#include "./passkeys/fully_delete_action.h"
#include "./passkeys/fully_delete_idle.h"
#include "./passkeys/initial_build_action_root.h"
#include "./passkeys/push_idle_hierarchy_position_to_form.h"
#include "../idles.h"
#include "../../forms/IdleAnimation.h"

namespace dovah::datastores::impl::idles {
   idle_node::~idle_node() {
      if (this->datastore._check_is_clearing({}))
         //
         // If *everything* is gonna be deleted anyway, then there's no point in 
         // telling any given action node that we're being deleted.
         //
         return;

      auto _sever = [this](std::vector<action_node*>& list) {
         for (auto* action : list) {
            if (!action)
               continue;
            action->_on_idle_fully_deleted({}, *this);
         }
         list.clear();
      };
      _sever(this->action_root_candidacies.masters);
      _sever(this->action_root_candidacies.active);
   }

   action_node* idle_node::get_parent_action() const noexcept {
      if (this->parent)
         return nullptr;
      auto& pair = this->action_root_candidacies;
      {
         auto& list = pair.active;
         if (!list.empty())
            return list.back();
      }
      {
         auto& list = pair.masters;
         if (!list.empty())
            return list.back();
      }
      return nullptr;
   }

   void idle_node::_build_action_root_candidacy(passkeys::initial_build_action_root, action_node& action, bool is_active_file) {
      auto& pair = this->action_root_candidacies;
      auto& list = is_active_file ? pair.active : pair.masters;
      list.push_back(&action);
   }

   void idle_node::_on_action_fully_deleted(passkeys::fully_delete_action, action_node& action) {
      auto _sever = [&action](std::vector<action_node*>& list) {
         bool any_severed = false;
         for (auto*& v : list) {
            if (v == &action) {
               v = nullptr;
               any_severed = true;
            }
         }
         if (any_severed)
            std::erase(list, nullptr);
      };
      _sever(this->action_root_candidacies.masters);
      _sever(this->action_root_candidacies.active);
   }

   void idle_node::_clear_active_action_root_candidacies(passkeys::action_update_root, action_node& action) {
      auto& list        = this->action_root_candidacies.active;
      bool  any_severed = false;
      for (auto*& v : list) {
         if (v == &action) {
            v = nullptr;
            any_severed = true;
         }
      }
      if (any_severed)
         std::erase(list, nullptr);
   }
   void idle_node::_add_active_action_root_candidacy(passkeys::action_update_root, action_node& action) {
      this->action_root_candidacies.active.push_back(&action);
   }

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
         }
      } else {
         i = 0;
         //

         static_assert(false, "TODO: This won't work given the order of operations for moving a child IDLE from its parent IDLE to an AACT");

         auto* parent_action = this->get_parent_action();
         if (parent_action)
            parent_stub = &parent_action->stub;
      }
         
      if (auto& cb = this->datastore.callbacks.on_any_form_modified.before)
         cb(this->stub);
      //
      if (parent_stub) {
         const auto& graph = loaded->get_behavior_graph_path(false);
         if (parent_stub->form_type == form_type::action) {
            loaded->make_action_root(graph, *parent_stub);
         } else {
            assert(parent_node != nullptr && "If the parent stub exists and isn't an AACT, it must be an IDLE. Why don't we have a parent list node?");
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

   void idle_node::_on_become_child_of_idle();
}