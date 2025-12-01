#include "./graph_node.h"
#include "./action_node.h"
#include "./loose_idle_list_node.h"

namespace dovah::datastores::impl::idles2 {
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

   const action_node* graph_node::get_action(form_stub& stub) const noexcept {
      for(const action_node* action : this->actions)
         if (&action->stub == &stub)
            return action;
      return nullptr;
   }
   action_node* graph_node::get_action(form_stub& stub) noexcept {
      return const_cast<action_node*>(std::as_const(*this).get_action(stub));
   }

   action_node* graph_node::get_or_create_action(form_stub& stub) {
      auto* action = this->get_action(stub);
      if (action)
         return action;

      action = new action_node(stub);
      action->graph = this;
      this->actions.push_back(action); // TODO: sorted insertion
      return action;
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