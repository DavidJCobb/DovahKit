#include "./action_node.h"
#include "./idle_node.h"
#include "./passkeys/action_update_root.h"
#include "./passkeys/check_is_clearing.h"
#include "./passkeys/fully_delete_action.h"
#include "./passkeys/fully_delete_idle.h"
#include "./passkeys/initial_build_action_root.h"
#include "../idles.h"
#include "../../files/file_load_order.h"

namespace dovah::datastores::impl::idles {
   action_node::~action_node() {
      if (this->datastore._check_is_clearing({}))
         //
         // If *everything* is gonna be deleted anyway, then there's no point in 
         // telling any given idle node that we're being deleted.
         //
         return;

      auto _sever = [this](std::vector<tracked_candidacy>& list) {
         for (auto& candidacy : list) {
            auto* node = candidacy.candidate;
            if (!node)
               continue;
            node->_on_action_fully_deleted({}, *this);
         }
         list.clear();
      };
      _sever(this->action_root_candidacies.masters);
      _sever(this->action_root_candidacies.active);
   }

   void action_node::set_active_root(idle_node& idle) {
      auto* prior_winner = this->get_winning_root_idle();

      auto& list = this->action_root_candidacies.active;
      for (auto& item : list) {
         if (!item.candidate)
            continue;
         item.candidate->_clear_active_action_root_candidacies({}, *this);
      }
      list.clear();

      auto& item = list.emplace_back();
      item.candidate = &idle;
      idle._add_active_action_root_candidacy({}, *this);

      if (prior_winner != &idle) {
         auto& cb = this->datastore.callbacks.on_action_root_changed;
         if (cb)
            (cb)(*this, idle);
      }
   }
   void action_node::unset_active_root(idle_node& idle) {
      auto* prior_winner = this->get_winning_root_idle();

      bool  severed = false;
      auto& list    = this->action_root_candidacies.active;
      for (auto& item : list) {
         if (item.candidate == &idle) {
            item.candidate = nullptr;
            severed = true;
         }
      }
      if (severed)
         std::erase_if(list, [](auto& item) { return item.candidate == nullptr; });
      else
         return;

      auto* new_winner = this->get_winning_root_idle();
      if (prior_winner != new_winner && new_winner) {
         auto& cb = this->datastore.callbacks.on_action_root_changed;
         if (cb)
            (cb)(*this, *new_winner);
      }
   }

   void action_node::_on_idle_fully_deleted(passkeys::fully_delete_idle, idle_node& idle) {
      bool killing_the_winner = this->get_winning_root_idle() == &idle;

      auto _sever = [&idle](std::vector<tracked_candidacy>& list) {
         bool any_severed = false;
         for (auto& candidacy : list) {
            if (candidacy.candidate == &idle) {
               candidacy.candidate = nullptr;
               any_severed = true;
            }
         }
         if (any_severed)
            std::erase_if(list, [](auto& item) { return item.candidate == nullptr; });
         return any_severed;
      };
      _sever(this->action_root_candidacies.masters);
      _sever(this->action_root_candidacies.active);

      if (killing_the_winner) {
         auto* new_winner = this->get_winning_root_idle();
         if (new_winner) {
            auto& cb = this->datastore.callbacks.on_action_root_changed;
            if (cb)
               (cb)(*this, *new_winner);
         }
      }
   }

   void action_node::_register_root_idle(passkeys::initial_build_action_root, const file_load_order& flo, idle_node& idle, const action_root_candidacy& info);
}