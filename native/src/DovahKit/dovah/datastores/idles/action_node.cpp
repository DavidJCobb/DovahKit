#include "./action_node.h"
#include <cassert>
#include "./passkeys/initial_build.h"
#include "./passkeys/post_build_edit.h"

namespace dovah::datastores::impl::idles {
   action_node::action_node(datastore_type& d, form_stub& stub) : node(d), stub(stub) {
   }

   bool action_node::candidates_include(const idle_node& idle) const noexcept {
      for (auto& cnd : this->candidacies.masters)
         if (cnd.idle == &idle)
            return true;
      for (auto& cnd : this->candidacies.active)
         if (cnd.idle == &idle)
            return true;
      return false;
   }

   void action_node::_track_candidate(passkeys::initial_build, const action_root_candidacy& cnd, idle_node& idle, bool via_master) {
      candidacy v = { cnd, &idle };

      auto& list      = via_master ? this->candidacies.masters : this->candidacies.active;
      auto  insert_at = std::upper_bound(list.begin(), list.end(), v);
      list.insert(insert_at, v);
   }

   // post-build:
   void action_node::_untrack_active_file_candidate(passkeys::post_build_edit, idle_node& idle) {
      auto& list = this->candidacies.active;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.idle == &idle) {
            list.erase(list.begin() + i);
            --size;
            --i;
         }
      }
   }
   void action_node::_track_active_file_candidate(passkeys::post_build_edit, idle_node& idle) {
      candidacy v = {
         {
            .offsets = {
               .of_record    = std::numeric_limits<size_t>::max(),
               .of_subrecord = std::numeric_limits<size_t>::max(),
            },
         },
         &idle,
      };
      auto& list      = this->candidacies.active;
      auto  insert_at = std::upper_bound(list.begin(), list.end(), v);
      list.insert(insert_at, v);
   }
   void action_node::_recalc_winning_root(passkeys::post_build_edit) {
      if (this->candidacies.active.empty() && this->candidacies.masters.empty()) {
         this->winning_root = nullptr;
         return;
      }
      auto& list = this->candidacies.active.empty() ? this->candidacies.masters : this->candidacies.active;
      this->winning_root = list.back().idle;
   }
}