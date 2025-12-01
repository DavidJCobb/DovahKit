#pragma once
#include "./action_node.h"

namespace dovah::datastores::impl::idles {
   constexpr idle_node* action_node::get_winning_root_idle() noexcept {
      return const_cast<idle_node*>(std::as_const(*this).get_winning_root_idle());
   }
   constexpr const idle_node* action_node::get_winning_root_idle() const noexcept {
      auto& pair = this->action_root_candidacies;
      {
         auto& list = pair.active;
         if (!list.empty())
            return list.back().candidate;
      }
      {
         auto& list = pair.masters;
         if (!list.empty())
            return list.back().candidate;
      }
      return nullptr;
   }
}