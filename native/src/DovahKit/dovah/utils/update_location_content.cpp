#include "./update_location_content.h"
#include "../data/hardcoded_form_ids.h"
#include "../form_stubs/helpers/for_each_child_form.h"
#include "../form_stubs/helpers/get_assigned_location.h"
#include "../form_stubs/helpers/get_base_form.h"
#include "../form_stubs/helpers/get_location_ref_type.h"
#include "../form_stubs/helpers/get_worldspace_persistent_cell.h"
#include "../form_stubs/helpers/is_persistent.h"
#include "../forms/ActorBase.h"
#include "../forms/DefaultObjectManager.h"
#include "../forms/Location.h"
#include "../forms/ObjectReference.h"
#include "../forms/components/extra_data/types/e/enable_state_parent.h"
#include "./get_computed_location.h"

namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}

namespace dovah::utils {
   void update_location_content::_crawl_special_refs(const form_stub& cell_or_world) {
      if (cell_or_world.form_type == form_type::cell) {
         form_stub_helpers::for_each_child_form(cell_or_world, [this](form_stub& child) {
            if (!form_type_is_reference(child.form_type))
               return;
            if (child.is_deleted())
               return;
            if (!_get_loc_ref_type(child))
               return;
            this->content.special_refs.push_back(&child);
         });
         return;
      }
      if (cell_or_world.form_type == form_type::worldspace) {
         //
         // We only process non-persistent-flagged cells inside of the worldspace. I assume 
         // this is because persistent-flagged cells should only contain persistent-flagged 
         // refs, which wouldn't need to be cached by LCTN... but that's just a guess.
         //
         auto* pcell = form_stub_helpers::get_worldspace_persistent_cell(cell_or_world);
         if (pcell && !form_stub_helpers::is_persistent(*pcell)) {
            if (!this->location || get_computed_location(*pcell) == this->location)
               this->_crawl_special_refs(*pcell);
         }
         form_stub_helpers::for_each_child_form(cell_or_world, [this](const form_stub& cell) {
            if (cell.form_type != form_type::cell)
               return;
            if (form_stub_helpers::is_persistent(cell))
               return;
            if (!this->location || get_computed_location(cell) == this->location)
               this->_crawl_special_refs(cell);
         });
         return;
      }
   }
   /*static*/ form_stub* update_location_content::_get_explicit_location(const form_stub& form) {
      auto* loc = form_stub_helpers::get_assigned_location(form);
      if (loc && loc->form_type == dovah::form_type::location)
         return loc;
      return nullptr;
   }
   /*static*/ form_stub* update_location_content::_get_loc_ref_type(const form_stub& ref) {
      auto* lrt = form_stub_helpers::get_location_ref_type(ref);
      if (lrt && lrt->form_type == form_type::location_ref_type)
         return lrt;
      return nullptr;
   }
   /*static*/ form_stub* update_location_content::_get_containing_world(const form_stub& cell) {
      if (cell.form_type != form_type::cell)
         return nullptr;
      auto* stub = cell.get_parent_form();
      if (stub && stub->form_type == form_type::worldspace)
         return stub;
      return nullptr;
   }

   std::pair<dovah::form_stub*, uint8_t> update_location_content::_get_enable_parent_info(form_stub& refr) { // parent and flags
      dovah::loaded_form_ptr<loaded_forms::ObjectReference> loaded_refr;
      if (this->_is_mid_save) {
         loaded_refr = refr.load_even_if_unsafe({}).ptr_cast<loaded_forms::ObjectReference>();
      } else {
         loaded_refr = refr.load().ptr_cast<loaded_forms::ObjectReference>();
      }
      if (!loaded_refr)
         return { nullptr, 0 };
      auto* extra = loaded_refr->extra_data.get<extra_data_types::enable_state_parent>();
      if (!extra)
         return { nullptr, 0 };
      return { extra->ref.get_form_stub(), extra->flags };
   };
   bool update_location_content::_is_unique_actor(form_stub& base_form) {
      if (base_form.form_type != form_type::actor_base)
         return false;
      dovah::loaded_form_ptr<loaded_forms::ActorBase> loaded;
      if (this->_is_mid_save) {
         //
         // Loading is blocked mid-save; this bypasses the block.
         //
         loaded = base_form.load_even_if_unsafe(form_stub_passkeys::force_form_load{}).ptr_cast<loaded_forms::ActorBase>();
      } else {
         loaded = base_form.load().ptr_cast<loaded_forms::ActorBase>();
      }
      if (!loaded)
         return false;
      return (loaded->actor_flags & loaded_forms::ActorBase::actor_flag::unique) != 0;
   }

   void update_location_content::_recache_notable_forms(file_load_order& lo) {
      this->_cache = {};
      this->_cache.NoZoneZone = lo.get_form_of_probable_type(form_type::encounter_zone, hardcoded_form_ids::NoZoneZone);
      {  // PersistLoc
         auto* dobj_man = lo.get_canonical_instance_of_singleton_form(form_type::default_object_manager);
         if (dobj_man) {
            auto loaded = dobj_man->load().ptr_cast<loaded_forms::DefaultObjectManager>();
            if (loaded) {
               this->_cache.PersistLoc = loaded->get_entry('PLOC');
            }
         }
      }
   }
   void update_location_content::_set_is_mid_save_location_fixup(location_update_during_save_passkey) {
      this->_is_mid_save = true;
   }
   void update_location_content::gather(form_stub& location) {
      if (location.form_type != form_type::location) {
         this->_cache   = {};
         this->location = nullptr;
         return;
      }
      this->_recache_notable_forms(location.get_owning_load_order());
      this->location = &location;

      if (&location == this->_cache.PersistLoc) {
         return;
      }

      for (auto& inbound : location.inbound) {
         auto* stub = inbound.second.other;
         if (!stub)
            continue;
         if (form_type_is_reference(stub->form_type)) {
            if (stub->is_deleted())
               continue;
            if (_get_explicit_location(*stub) != &location)
               continue;
            this->content.persist_loc_refs.push_back(stub); // *CSR

            auto* base = form_stub_helpers::get_base_form(*stub);
            if (base && !base->is_deleted() && _is_unique_actor(*base))
               this->content.unique_actors.push_back(stub); // *CUN
         } else if (stub->form_type == form_type::cell) {
            this->_crawl_special_refs(*stub); // *CSR
            //
            // Below: Criteria for inclusion in *CEC.
            //
            if (!stub->is_exterior_cell())
               continue;
            if (get_computed_location(*stub) != &location)
               continue;
            auto* world = _get_containing_world(*stub);
            if (!world || get_computed_location(*world) == &location) // only include a cell if the entire containing world isn't in the location
               continue;
            //
            // Below: *CEC.
            //
            bool world_present = false;
            for (auto& item : this->content.exterior_cell_lists) {
               if (item.worldspace == world) {
                  world_present = true;

                  bool cell_present = false;
                  for (auto* cell : item.cells) {
                     if (cell == stub) {
                        cell_present = true;
                        break;
                     }
                  }
                  if (!cell_present) {
                     item.cells.push_back(stub);
                  }

                  break;
               }
            }
            if (!world_present) {
               auto& item = this->content.exterior_cell_lists.emplace_back();
               item.worldspace = world;
               item.cells.push_back(stub);
            }
         } else if (stub->form_type == form_type::worldspace) {
            this->_crawl_special_refs(*stub); // *CSR
         }
      }
   }

   void update_location_content::apply(bool as_base_record) {
      if (!this->location)
         return;

      auto  loaded_sp = this->location->load().ptr_cast<loaded_forms::Location>();
      auto* loaded    = loaded_sp.unwrap();
      assert(loaded != nullptr);

      this->_apply_persist_loc_refs(*loaded, as_base_record); // *CPR and by extension, *CEP and *CID
      this->_apply_unique_actors(*loaded, as_base_record); // *CUN
      this->_apply_special_refs(*loaded, as_base_record); // *CSR
      this->_apply_exterior_cells(*loaded, as_base_record);
   }

   namespace {
      static void _clear_list(loaded_forms::Location& form, std::vector<loaded_forms::Location::enable_parent>& list) {
         for (auto& item : list) {
            item.ref.set(form, nullptr);
            item.enable_parent.set(form, nullptr);
         }
         list.clear();
      }
      static void _clear_list(loaded_forms::Location& form, std::vector<loaded_forms::Location::persist_loc_ref>& list) {
         for (auto& item : list) {
            item.ref.set(form, nullptr);
            item.cell_or_world.set(form, nullptr);
         }
         list.clear();
      }
      static void _clear_list(loaded_forms::Location& form, std::vector<loaded_forms::Location::special_ref>& list) {
         for (auto& item : list) {
            item.reference.set(form, nullptr);
            item.ref_type.set(form, nullptr);
            item.cell_or_world.set(form, nullptr);
         }
         list.clear();
      }
      static void _clear_list(loaded_forms::Location& form, std::vector<loaded_forms::Location::unique_actor>& list) {
         for (auto& item : list) {
            item.actor.set(form, nullptr);
            item.actor_base.set(form, nullptr);
            item.editor_location.set(form, nullptr);
         }
         list.clear();
      }
   }
   void update_location_content::_apply_persist_loc_refs(loaded_forms::Location& loaded, bool as_base_record) {
      //
      // First, clear the lists.
      //
      _clear_list(loaded, loaded.contents.persistent_refs.full);
      _clear_list(loaded, loaded.contents.enable_parents.full);
      clear_form_reference_list(loaded.contents.initially_disabled.full, loaded);
      if (as_base_record) {
         _clear_list(loaded, loaded.contents.persistent_refs.base);
         _clear_list(loaded, loaded.contents.enable_parents.base);
         clear_form_reference_list(loaded.contents.initially_disabled.base, loaded);
      }

      auto& dataset  = loaded.contents.persistent_refs;
      auto& gathered = this->content.persist_loc_refs;

      auto _append_persist_loc_ref = [&loaded](std::vector<loaded_forms::Location::persist_loc_ref>& list, form_stub& stub) {
         auto& item = list.emplace_back();
         item.ref.set(loaded, &stub);

         auto* cell  = stub.get_parent_form();
         auto* world = _get_containing_world(*cell);
         if (world) {
            item.cell_or_world.set(loaded, world);
            int32_t x = 0;
            int32_t y = 0;
            cell->get_grid_coordinates(x, y);
            item.grid.x = x;
            item.grid.y = y;
         } else {
            item.cell_or_world.set(loaded, cell);
            item.grid.x = 0x7FFF;
            item.grid.y = 0x7FFF;
         }
      };
      
      auto _is_initially_disabled = [](const form_stub& refr) -> bool {
         return refr.test_record_flags(loaded_forms::ObjectReference::form_flag::disabled);
      };

      if (as_base_record) {
         for (auto* stub : gathered) {
            _append_persist_loc_ref(dataset.base, *stub);
            _append_persist_loc_ref(dataset.full, *stub);
            //
            // Update "initially disabled" and "enable parents" lists.
            //
            if (_is_initially_disabled(*stub)) {
               loaded.contents.initially_disabled.base.emplace_back().set(loaded, stub);
               loaded.contents.initially_disabled.full.emplace_back().set(loaded, stub);
            }
            if (auto ep = _get_enable_parent_info(*stub); ep.first) {
               {
                  auto& item = loaded.contents.enable_parents.base.emplace_back();
                  item.ref.set(loaded, stub);
                  item.enable_parent.set(loaded, ep.first);
                  item.flags = ep.second;
               }
               {
                  auto& item = loaded.contents.enable_parents.full.emplace_back();
                  item.ref.set(loaded, stub);
                  item.enable_parent.set(loaded, ep.first);
                  item.flags = ep.second;
               }
            }
         }
      } else {
         for (auto* stub : gathered) {
            _append_persist_loc_ref(dataset.full, *stub);
            //
            // Update "initially disabled" and "enable parents" lists.
            //
            if (_is_initially_disabled(*stub)) {
               loaded.contents.initially_disabled.full.emplace_back().set(loaded, stub);
            }
            if (auto ep = _get_enable_parent_info(*stub); ep.first) {
               auto& item = loaded.contents.enable_parents.full.emplace_back();
               item.ref.set(loaded, stub);
               item.enable_parent.set(loaded, ep.first);
               item.flags = ep.second;
            }
         }
      }
   }
   void update_location_content::_apply_exterior_cells(loaded_forms::Location& loaded, bool as_base_record) {
      using backend_item_type  = loaded_forms::Location::exterior_cell_list;
      using pending_item_type = exterior_cell_list;

      using grid_coords = loaded_forms::Location::grid_coords;

      auto& dataset  = loaded.contents.exterior_cells;
      auto& gathered = this->content.exterior_cell_lists;

      auto _clear = [&loaded](std::vector<backend_item_type>& list) {
         for (auto& item : list)
            item.worldspace.set(loaded, nullptr);
         list.clear();
      };
      auto _append = [&loaded](std::vector<backend_item_type>& list, pending_item_type& ecl) {
         auto& item = list.emplace_back();
         item.worldspace.set(loaded, ecl.worldspace);
         for (auto* cell : ecl.cells) {
            int32_t gx = 0;
            int32_t gy = 0;
            cell->get_grid_coordinates(gx, gy);
            if (gx < std::numeric_limits<int16_t>::lowest() || gx > std::numeric_limits<int16_t>::max())
               continue;
            if (gy < std::numeric_limits<int16_t>::lowest() || gy > std::numeric_limits<int16_t>::max())
               continue;
            item.cells.emplace_back(grid_coords{
               .y = (int16_t)gy,
               .x = (int16_t)gx,
            });
         }
      };

      _clear(dataset.full);
      if (as_base_record) {
         _clear(dataset.base);
         for (auto& pending_item : gathered) {
            _append(dataset.base, pending_item);
            _append(dataset.full, pending_item);
         }
      } else {
         for (auto& pending_item : gathered) {
            _append(dataset.full, pending_item);
         }
      }
   }
   void update_location_content::_apply_special_refs(loaded_forms::Location& loaded, bool as_base_record) {
      using grid_coords = loaded_forms::Location::grid_coords;

      auto& dataset  = loaded.contents.special_refs;
      auto& gathered = this->content.special_refs;

      auto _get_placement = [](const form_stub& refr) -> std::pair<form_stub*, grid_coords> { // returns cell-or-world + grid
         auto* cell  = refr.get_parent_form();
         auto* world = _get_containing_world(*cell);
         if (world) {
            int32_t gx = 0;
            int32_t gy = 0;
            cell->get_grid_coordinates(gx, gy);
            return { world, { .y = (int16_t)gy, .x = (int16_t)gx } };
         }
         return { cell, { 0x7FFF, 0x7FFF } };
      };

      _clear_list(loaded, dataset.full);
      if (as_base_record) {
         _clear_list(loaded, dataset.base);
         for (auto* refr : gathered) {
            auto* ref_type  = _get_loc_ref_type(*refr);
            auto  placement = _get_placement(*refr);
            {
               auto& item = dataset.base.emplace_back();
               item.reference.set(loaded, refr);
               item.ref_type.set(loaded, ref_type);
               item.cell_or_world.set(loaded, placement.first);
               item.grid = placement.second;
            }
            {
               auto& item = dataset.full.emplace_back();
               item.reference.set(loaded, refr);
               item.ref_type.set(loaded, ref_type);
               item.cell_or_world.set(loaded, placement.first);
               item.grid = placement.second;
            }
         }
      } else {
         for (auto* refr : gathered) {
            auto* ref_type  = _get_loc_ref_type(*refr);
            auto  placement = _get_placement(*refr);
            {
               auto& item = dataset.full.emplace_back();
               item.reference.set(loaded, refr);
               item.ref_type.set(loaded, ref_type);
               item.cell_or_world.set(loaded, placement.first);
               item.grid = placement.second;
            }
         }
      }
   }
   void update_location_content::_apply_unique_actors(loaded_forms::Location& loaded, bool as_base_record) {
      auto& dataset  = loaded.contents.unique_actors;
      auto& gathered = this->content.unique_actors;

      auto _get_editor_location = [](const dovah::form_stub& refr) {
         form_stub* editor_loc = nullptr;
         form_stub* cell       = nullptr;
         cell = refr.get_parent_form();
         if (cell && cell->form_type != form_type::cell)
            cell = nullptr;
         /*
            CK logic for unique actor editor locations is as follows:

               BGSLocation* TESObjectREFR::GetEditorLocationForm() {
                  if (this->parentCell)
                     return this->parentCell->GetComputedLocation();

                  // if not persistent:                         parent cell
                  // if persistent and parent cell is interior: parent cell
                  // if persistent and parent cell is exterior: worldspace persistent cell
                  TESObjectCELL* parent_cell = static_cast<TESChildCell*>(this)->Unk_01();

                  if (parent_cell) {
                     auto* world = parent_cell->GetCurrentContainingWorld();
                     if (world)
                        return world->GetComputedLocation();
                  }
                  return this->extraData.GetLocation();
               }

            Can't figure out when or how a ref would have no parent cell, for most of this 
            behavior to come into play.
         */
         if (cell)
            return get_computed_location(*cell);
         return _get_explicit_location(refr);
      };

      _clear_list(loaded, dataset.full);
      if (as_base_record) {
         _clear_list(loaded, dataset.base);
         for (auto* refr : gathered) {
            form_stub* base_form  = form_stub_helpers::get_base_form(*refr);
            form_stub* editor_loc = _get_editor_location(*refr);
            {
               auto& item = dataset.base.emplace_back();
               item.actor.set(loaded, refr);
               item.actor_base.set(loaded, base_form);
               item.editor_location.set(loaded, editor_loc);
            }
            {
               auto& item = dataset.full.emplace_back();
               item.actor.set(loaded, refr);
               item.actor_base.set(loaded, base_form);
               item.editor_location.set(loaded, editor_loc);
            }
         }
      } else {
         for (auto* refr : gathered) {
            form_stub* base_form  = form_stub_helpers::get_base_form(*refr);
            form_stub* editor_loc = _get_editor_location(*refr);
            {
               auto& item = dataset.full.emplace_back();
               item.actor.set(loaded, refr);
               item.actor_base.set(loaded, base_form);
               item.editor_location.set(loaded, editor_loc);
            }
         }
      }
   }
}