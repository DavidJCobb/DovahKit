#include "form_stub_helpers.h"
#include "form_stub.h"
#include "form_stub_addenda.h"
#include "files/common.h"

namespace dovah::form_stub_helpers {
   void for_each_child_form(const form_stub* parent, std::function<bool(form_stub*)> functor) {
      if (!(form_type_info::lookup(parent->formType).flags & form_type_info::flag::can_have_children))
         return;
      for (auto& pair : parent->inbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::parent_child) {
            auto* child = entry.other;
            if (!child)
               continue;
            if (functor(child))
               break;
         }
      }
   }
   void for_each_quest_topic(const form_stub* quest, std::function<bool(form_stub*)> functor) {
      if (quest->formType != form_type::quest)
         return;
      for (auto& pair : quest->inbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::dialogue_quest) {
            auto* child = entry.other;
            if (!child || child->formType != form_type::topic)
               continue;
            if (functor(child))
               break;
         }
      }
   }
   form_stub* get_base_form(const form_stub* ref) {
      if (!form_type_info::form_type_is_reference(ref->formType))
         return nullptr;
      for (auto& pair : ref->outbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::object_reference)
            return entry.other;
      }
      return nullptr;
   }
   form_stub* get_worldspace_persistent_cell(const form_stub* world) {
      if (!world->addenda)
         return nullptr;
      return world->addenda->persistent_cell;
   }
   form_stub* get_worldspace_cell_by_grid(const form_stub* world, int32_t x, int32_t y) {
      const form_stub* persistent_cell = get_worldspace_persistent_cell(world);
      for (auto& pair : world->inbound) {
         auto& entry = pair.second;
         if (!(entry.flags & use_info_entry::flag::parent_child))
            continue;
         auto* cell = entry.other;
         if (!cell || cell->formType != form_type::cell)
            continue;
         assert(cell->get_parent_form() == world && "How did a worldspace form a parent/child relationship with a cell that doesn't consider that world its parent?");
         if (cell == persistent_cell)
            continue;
         if (!cell->addenda)
            continue;
         auto& g = cell->addenda->grid_coords;
         if (g.x == x && g.y == y)
            return cell;
      }
      return nullptr;
   }
   extern form_stub* get_cell_landscape(const form_stub* cell) {
      if (cell->formType != dovah::form_type::cell)
         return nullptr;
      if (auto* addenda = cell->addenda)
         if (auto* land = addenda->canonical_landscape)
            return land;
      for (auto& pair : cell->inbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::parent_child) {
            auto* child = entry.other;
            if (!child)
               continue;
            if (child->formType == dovah::form_type::land)
               return child;
         }
      }
      return nullptr;
   }

   extern bool is_persistent(const form_stub* stub) {
      if (!stub)
         return false;
      if (form_type_info::form_type_is_reference(stub->formType)) {
         return stub->test_record_flags(0x400);
      }
      return false;
   }
   extern void for_each_persistent_ref_in_world(const form_stub& world, std::function<bool(form_stub*)> functor) {
      if (world.formType != form_type::worldspace)
         return;
      for_each_child_form(&world, [functor](form_stub* cell) {
         bool result;
         if (cell->formType != form_type::cell)
            return false;
         for_each_child_form(cell, [functor, &result](form_stub* child) {
            if (!is_persistent(child))
               return false;
            result = (functor)(child);
            return result;
         });
         return result;
      });
   }

   extern bool is_worldspace_persistent_cell(const form_stub& cell) {
      auto* parent = cell.get_parent_form();
      if (!parent)
         return false;
      if (parent->formType != form_type::worldspace)
         return false;
      return &cell == get_worldspace_persistent_cell(parent);
   }
}