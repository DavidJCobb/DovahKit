#include "form_stub_helpers.h"
#include "form_stub.h"
#include "files/common.h"

namespace dovah::form_stub_helpers {
   void for_each_child_form(const form_stub* parent, std::function<bool(form_stub*)> functor) {
      for (auto& pair : parent->outbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::i_am_parent_of) {
            auto* child = entry.other;
            if (!child)
               continue;
            if (functor(child))
               break;
         }
      }
   }
   form_stub* get_worldspace_persistent_cell(const form_stub* world) {
      for (auto& pair : world->outbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::i_am_parent_of) {
            auto* child = entry.other;
            if (!child)
               continue;
            if (child->groupInfo.type != (int)tes_file_group_type::world_children)
               continue;
            if (child->formType != form_type::cell)
               continue;
            return child;
         }
      }
      return nullptr;
   }
}