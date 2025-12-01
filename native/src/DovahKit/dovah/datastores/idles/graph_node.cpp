#include "./graph_node.h"
#include <cassert>
#include "helpers/vectors/re_sort_item_within.h"
#include "./action_node.h"
#include "./loose_idle_list_node.h"
#include "./passkeys/initial_build.h"
#include "./passkeys/post_build_edit.h"
#include "../../form_stub.h"

namespace dovah::datastores::impl::idles {
   graph_node::graph_node(datastore_type& d, std::string_view path) : node(d), path(path) {
      this->loose = new loose_idle_list_node(d);
      this->loose->graph = this;
   }
   graph_node::~graph_node() {
      {
         auto& list = this->actions;
         for (auto& ptr : list) {
            delete (action_node*)ptr;
            ptr = nullptr;
         }
         list.clear();
      }
      delete this->loose;
      this->loose = nullptr;
   }

   namespace {
      bool _compare_actions(const form_stub& a, form_stub& b) {
         const std::string_view name_a = a.get_editor_id();
         const std::string_view name_b = b.get_editor_id();
         const auto min_size = std::min(name_a.size(), name_b.size());
         for (size_t i = 0; i < min_size; ++i) {
            char ca = name_a[i];
            char cb = name_b[i];
            if (ca >= 'A' && ca <= 'Z')
               ca += 0x20;
            if (cb >= 'A' && cb <= 'Z')
               cb += 0x20;
            if (ca != cb)
               return ca < cb;
         }
         return name_b.size() > min_size;
      }
   }
   /*static*/ bool graph_node::action_sort_comparator(const action_node* a, const action_node* b) {
      return _compare_actions(a->stub, b->stub);
   }

   const action_node* graph_node::get_action(form_stub& stub) const noexcept {
      for(const action_node* action : this->actions)
         if (&action->stub == &stub)
            return action;
      return nullptr;
   }
   action_node* graph_node::get_action(form_stub& stub) noexcept {
      return const_cast<action_node*>(std::as_const(*this).get_action(stub));
   }

   size_t graph_node::index_of_action(const action_node& action) const noexcept {
      for (size_t i = 0; i < this->actions.size(); ++i)
         if (this->actions[i] == &action)
            return i;
      return index_of_none;
   }
   size_t graph_node::index_of_action(const form_stub& stub) const noexcept {
      for (size_t i = 0; i < this->actions.size(); ++i)
         if (&this->actions[i]->stub == &stub)
            return i;
      return index_of_none;
   }

   size_t graph_node::prospective_index_of(const action_node& action) const noexcept {
      auto it = std::upper_bound(
         this->actions.begin(),
         this->actions.end(),
         &action,
         &action_sort_comparator
      );
      return std::distance(this->actions.begin(), it);
   }
   size_t graph_node::prospective_index_of(const form_stub& stub) const noexcept {
      auto it = std::upper_bound(
         this->actions.begin(),
         this->actions.end(),
         &stub,
         [](const dovah::form_stub* a, const action_node* b) {
            return _compare_actions(*a, b->stub);
         }
      );
      return std::distance(this->actions.begin(), it);
   }

   action_node* graph_node::get_or_create_action(form_stub& stub) {
      auto* action = this->get_action(stub);
      if (action)
         return action;

      action = new action_node(this->datastore, stub);
      action->graph = this;
      this->actions.push_back(action); // TODO: sorted insertion
      return action;
   }

   void graph_node::re_sort_all_actions(passkeys::initial_build) {
      std::sort(
         this->actions.begin(),
         this->actions.end(),
         &action_sort_comparator
      );
   }
   void graph_node::re_sort_action(passkeys::post_build_edit, size_t i) {
      assert(i < this->actions.size());
      cobb::vectors::re_sort_item_within(
         this->actions,
         this->actions.begin() + i,
         &action_sort_comparator
      );
   }

   bool graph_node::path_equals(std::string_view other) const noexcept {
      //
      // The game stores these paths as BSFixedStrings (i.e. interned strings with 
      // case-folding).
      //
      const auto size = this->path.size();
      if (size != other.size())
         return false;
      for (size_t i = 0; i < size; ++i) {
         char a = this->path[i];
         char b = other[i];
         if (a >= 'A' && a <= 'Z')
            a += 0x20;
         if (b >= 'A' && b <= 'Z')
            b += 0x20;
         if (a != b)
            return false;
      }
      return true;
   }
}