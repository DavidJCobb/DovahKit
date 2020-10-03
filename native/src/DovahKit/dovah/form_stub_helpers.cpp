#include "form_stub_helpers.h"
#include "form_stub.h"
#include "files/common.h"

namespace dovah::form_stub_helpers {
   void for_each_child_form(const form_stub* parent, std::function<bool(form_stub*)> functor) {
      if (!(form_type_info::lookup(parent->formType).flags & form_type_info::flag::can_have_children))
         return;
      for (auto& pair : parent->inbound) {
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
   form_stub* get_base_form(const form_stub* ref) {
      for (auto& pair : ref->outbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::i_am_reference_of)
            return entry.other;
      }
      return nullptr;
   }
   form_stub* get_worldspace_persistent_cell(const form_stub* world) {
      for (auto& pair : world->inbound) {
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
   form_stub* get_worldspace_cell_by_grid(const form_stub* world, int32_t x, int32_t y) {
      for (auto& pair : world->inbound) {
         auto& entry = pair.second;
         if (!(entry.flags & use_info_entry::flag::i_am_parent_of))
            continue;
         auto* cell = entry.other;
         if (!cell || cell->formType != form_type::cell)
            continue;
         assert(cell->groupInfo.parentFormID == world->formID && "How did a worldspace form a parent/child relationship with a cell that doesn't consider that world its parent?");
         if (cell->groupInfo.gridX == x && cell->groupInfo.gridY == y)
            return cell;
      }
      return nullptr;
   }
}