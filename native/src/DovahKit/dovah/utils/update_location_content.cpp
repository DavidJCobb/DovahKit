#include "./update_location_content.h"
#include "../data/hardcoded_form_ids.h"
#include "../form_stubs/helpers/for_each_child_form.h"
#include "../form_stubs/helpers/get_base_form.h"
#include "../form_stubs/helpers/get_worldspace_persistent_cell.h"
#include "../form_stubs/helpers/is_persistent.h"
#include "../forms/ActorBase.h"
#include "../forms/Cell.h"
#include "../forms/DefaultObjectManager.h"
#include "../forms/EncounterZone.h"
#include "../forms/Location.h"
#include "../forms/ObjectReference.h"
#include "../forms/Worldspace.h"
#include "../forms/components/extra_data/types/e/enable_state_parent.h"
#include "../forms/components/extra_data/types/e/encounter_zone.h"
#include "../forms/components/extra_data/types/l/location.h"
#include "../forms/components/extra_data/types/l/location_ref_type.h"
#include "./get_computed_location.h"

namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}

namespace dovah::utils {
   void update_location_content::_crawl_special_refs(form_stub& cell_or_world) {
      if (cell_or_world.form_type == form_type::cell) {
         form_stub_helpers::for_each_child_form(&cell_or_world, [this](form_stub* child) {
            if (!form_type_is_reference(child->form_type))
               return;
            if (child->is_deleted())
               return;
            if (!_get_loc_ref_type(*child))
               return;
            this->content.special_refs.push_back(child);
         });
         return;
      }
      if (cell_or_world.form_type == form_type::worldspace) {
         //
         // We only process non-persistent-flagged cells inside of the worldspace. I assume 
         // this is because persistent-flagged cells should only contain persistent-flagged 
         // refs, which wouldn't need to be cached by LCTN... but that's just a guess.
         //
         auto* pcell = form_stub_helpers::get_worldspace_persistent_cell(&cell_or_world);
         if (pcell && !form_stub_helpers::is_persistent(pcell)) {
            if (!this->location || get_computed_location(*pcell) == this->location)
               this->_crawl_special_refs(*pcell);
         }
         form_stub_helpers::for_each_child_form(&cell_or_world, [this](form_stub* cell) {
            if (cell->form_type != form_type::cell)
               return;
            if (form_stub_helpers::is_persistent(cell))
               return;
            if (!this->location || get_computed_location(*cell) == this->location)
               this->_crawl_special_refs(*cell);
         });
         return;
      }
   }
   /*static*/ form_stub* update_location_content::_get_encounter_zone(form_stub& form) {
      if (form.form_type == form_type::cell) {
         if (auto loaded = form.load().ptr_cast<loaded_forms::Cell>()) {
            auto* extra = loaded->extra_data.get<extra_data_types::encounter_zone>();
            if (extra && extra->form)
               return extra->form.get_form_stub();
         }
         if (auto* world = _get_containing_world(form))
            return _get_encounter_zone(*world);
      } else if (form.form_type == form_type::worldspace) {
         if (auto loaded = form.load().ptr_cast<loaded_forms::Worldspace>())
            return loaded->encounter_zone.get_form_stub();
      }
      return nullptr;
   }
   /*static*/ form_stub* update_location_content::_get_explicit_location(form_stub& form) {
      if (form_type_is_reference(form.form_type)) {
         auto loaded = form.load().ptr_cast<loaded_forms::ObjectReference>();
         if (!loaded)
            return nullptr;
         auto* extra = loaded->extra_data.get<extra_data_types::location>();
         if (!extra)
            return nullptr;
         return extra->form.get_form_stub();
      }
      switch (form.form_type) {
         case form_type::cell:
            {
               auto loaded = form.load().ptr_cast<loaded_forms::Cell>();
               if (!loaded)
                  break;
               auto* extra = loaded->extra_data.get<extra_data_types::location>();
               if (!extra)
                  break;
               return extra->form.get_form_stub();
            }
            break;
         case form_type::encounter_zone:
            {
               auto loaded = form.load().ptr_cast<loaded_forms::EncounterZone>();
               if (loaded)
                  return loaded->location.get_form_stub();
            }
            break;
         case form_type::worldspace:
            {
               auto loaded = form.load().ptr_cast<loaded_forms::Worldspace>();
               if (loaded)
                  return loaded->location.get_form_stub();
            }
            break;
      }
      return nullptr;
   }
   /*static*/ form_stub* update_location_content::_get_loc_ref_type(form_stub& ref) {
      auto loaded_refr = ref.load().ptr_cast<loaded_forms::ObjectReference>();
      if (!loaded_refr)
         return nullptr;
      auto* extra = loaded_refr->extra_data.get<extra_data_types::location_ref_type>();
      if (!extra)
         return nullptr;
      return extra->form.get_form_stub();
   }
   /*static*/ form_stub* update_location_content::_get_containing_world(form_stub& cell) {
      if (cell.form_type != form_type::cell)
         return nullptr;
      auto* stub = cell.get_parent_form();
      if (stub && stub->form_type == form_type::worldspace)
         return stub;
      return nullptr;
   }
   /*static*/ bool update_location_content::_is_unique_actor(form_stub& base_form) {
      if (base_form.form_type != form_type::actor_base)
         return false;
      auto loaded = base_form.load().ptr_cast<loaded_forms::ActorBase>();
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

            auto* base = form_stub_helpers::get_base_form(stub);
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
            if (!world || get_computed_location(*world) != &location)
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

      #pragma region Setup *CID and *CEP
      {
         auto _clear_cep = [loaded](auto& list) {
            for (auto& item : list)
               item.enable_parent.set(*loaded, nullptr);
            list.clear();
         };

         clear_form_reference_list(loaded->contents.initially_disabled.full, *loaded);
         _clear_cep(loaded->contents.enable_parents.full);
         if (as_base_record) {
            clear_form_reference_list(loaded->contents.initially_disabled.base, *loaded);
            _clear_cep(loaded->contents.enable_parents.base);
         }
      }
      #pragma endregion

      #pragma region *CPR and *CID and *CEP
      {
         auto& dataset  = loaded->contents.persistent_refs;
         auto& gathered = this->content.persist_loc_refs;

         auto _clear = [loaded](auto& list) {
            for (auto& item : list) {
               item.ref.set(*loaded, nullptr);
               item.cell_or_world.set(*loaded, nullptr);
            }
            list.clear();
         };
         auto _append = [loaded](auto& list, form_stub& stub) {
            auto& item = list.emplace_back();
            item.ref.set(*loaded, &stub);

            auto* cell  = stub.get_parent_form();
            auto* world = _get_containing_world(*cell);
            if (world) {
               item.cell_or_world.set(*loaded, world);
               int32_t x = 0;
               int32_t y = 0;
               cell->get_grid_coordinates(x, y);
               item.grid.x = x;
               item.grid.y = y;
            } else {
               item.cell_or_world.set(*loaded, cell);
               item.grid.x = 0x7FFF;
               item.grid.y = 0x7FFF;
            }
         };

         auto _process_initially_disabled = [loaded](auto& list, form_stub& refr) {
            if (refr.test_record_flags(loaded_forms::ObjectReference::form_flag::disabled)) {
               list.emplace_back().set(*loaded, &refr);
            }
         };
         auto _process_enable_parent = [loaded](auto& list, form_stub& refr) {
            auto loaded_refr = refr.load().ptr_cast<loaded_forms::ObjectReference>();
            if (!loaded_refr)
               return;
            auto* extra = loaded_refr->extra_data.get<extra_data_types::enable_state_parent>();
            if (!extra)
               return;
            auto& item = list.emplace_back();
            item.ref.set(*loaded, &refr);
            item.enable_parent.set(*loaded, extra->ref);
            item.flags = extra->flags;
         };

         _clear(dataset.full);
         if (as_base_record) {
            _clear(dataset.base);
            for (auto* stub : gathered) {
               _append(dataset.base, *stub);
               //
               // Update "initially disabled" and "enable parents" lists.
               //
               _process_initially_disabled(loaded->contents.initially_disabled.base, *stub);
               _process_enable_parent(loaded->contents.enable_parents.base, *stub);
            }
         } else {
            for (auto* stub : gathered) {
               _append(dataset.full, *stub);
               //
               // ACPR shouldn't contain any entries that are identical to LCPR.
               //
               auto& ACPR = dataset.full.back();
               bool  same = false;
               for (const auto& LCPR : dataset.base) {
                  if (LCPR == ACPR) {
                     same = true;
                     break;
                  }
               }
               if (same) {
                  dataset.full.pop_back();
               } else {
                  //
                  // Update "initially disabled" and "enable parents" lists.
                  //
                  _process_initially_disabled(loaded->contents.initially_disabled.full, *stub);
                  _process_enable_parent(loaded->contents.enable_parents.full, *stub);
               }
            }
         }
      }
      #pragma endregion
      #pragma region *CUN: Unique Actors
      {
         auto& dataset  = loaded->contents.unique_actors;
         auto& gathered = this->content.unique_actors;

         auto _clear = [loaded](auto& list) {
            for (auto& item : list) {
               item.actor.set(*loaded, nullptr);
               item.actor_base.set(*loaded, nullptr);
               item.editor_location.set(*loaded, nullptr);
            }
            list.clear();
         };
         auto _append = [loaded](auto& list, form_stub& stub) {
            auto& item = list.emplace_back();
            item.actor.set(*loaded, &stub);
            item.actor_base.set(*loaded, form_stub_helpers::get_base_form(&stub));
            //
            // Editor location:
            //
            form_stub* editor_loc = nullptr;
            {
               form_stub* cell = nullptr;
               cell = stub.get_parent_form();
               if (cell && cell->form_type != form_type::cell)
                  cell = nullptr;
               /*
                  The CK would check if the REFR has the "persistent" flag or flag 0x4000, 
                  and if the REFR's parent cell is nullptr or an exterior. If all of these 
                  conditions are met, the CK would grab the REFR's persistent cell via its 
                  extra-data.

                  That particular extra-data type isn't serialized to the file;  it exists 
                  only as run-time state,  and seems to be set when the REFR is added to a 
                  cell's REFR list.  (Not sure how it differs from the parent cell, then?)

                  (This is all done via TESChildCell's v-func 0x01.)

                  In any case, I think we can just skip that processing.
               */
               bool via_cell = false;
               if (cell) {
                  auto* world = _get_containing_world(*cell);
                  if (world) {
                     via_cell   = true;
                     editor_loc = _get_explicit_location(*world);
                  }
               }
               if (!via_cell)
                  editor_loc = _get_explicit_location(stub);
            }
            item.editor_location.set(*loaded, editor_loc);
         };

         _clear(dataset.full);
         if (as_base_record) {
            _clear(dataset.base);
            for (auto* stub : gathered)
               _append(dataset.base, *stub);
         } else {
            for (auto* stub : gathered)
               _append(dataset.full, *stub);
         }
      }
      #pragma endregion
      #pragma region *CSR: Special Refs
      {
         auto& dataset  = loaded->contents.special_refs;
         auto& gathered = this->content.special_refs;

         auto _clear = [loaded](auto& list) {
            for (auto& item : list) {
               item.reference.set(*loaded, nullptr);
               item.ref_type.set(*loaded, nullptr);
               item.cell_or_world.set(*loaded, nullptr);
            }
            list.clear();
         };
         auto _append = [loaded](auto& list, form_stub& stub) {
            auto& item = list.emplace_back();
            item.reference.set(*loaded, &stub);
            item.ref_type.set(*loaded, _get_loc_ref_type(stub));

            auto* cell  = stub.get_parent_form();
            auto* world = _get_containing_world(*cell);
            if (world) {
               item.cell_or_world.set(*loaded, world);
               int32_t x = 0;
               int32_t y = 0;
               cell->get_grid_coordinates(x, y);
               item.grid.x = x;
               item.grid.y = y;
            } else {
               item.cell_or_world.set(*loaded, cell);
               item.grid.x = 0x7FFF;
               item.grid.y = 0x7FFF;
            }
         };

         _clear(dataset.full);
         if (as_base_record) {
            _clear(dataset.base);
            for (auto* stub : gathered)
               _append(dataset.base, *stub);
         } else {
            for (auto* stub : gathered)
               _append(dataset.full, *stub);
         }
      }
      #pragma endregion
      #pragma region *CEC: Exterior Cells
      {
         auto& dataset  = loaded->contents.exterior_cells;
         auto& gathered = this->content.exterior_cell_lists;

         auto _clear = [loaded](auto& list) {
            for (auto& item : list) {
               item.worldspace.set(*loaded, nullptr);
            }
            list.clear();
         };
         auto _append = [loaded](auto& list, exterior_cell_list& ecl) {
            auto& item = list.emplace_back();
            item.worldspace.set(*loaded, ecl.worldspace);
            for (auto* cell : ecl.cells) {
               int32_t x = 0;
               int32_t y = 0;
               cell->get_grid_coordinates(x, y);
               
               auto& dst = item.cells.emplace_back();
               dst.x = x;
               dst.y = y;
            }
         };

         _clear(dataset.full);
         if (as_base_record) {
            _clear(dataset.base);
            for (auto& item : gathered)
               _append(dataset.base, item);
         } else {
            for (auto& item : gathered)
               _append(dataset.full, item);
         }
      }
      #pragma endregion
   }
}