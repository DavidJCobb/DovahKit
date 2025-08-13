#include "./Location.h"
#include "./_common_cpp.h"
#include "../../helpers/vector.h"

#include "../notices/form_load_warnings/by_form_type/location/base_record_should_not_have_content_removals.h"
#include "../notices/form_load_warnings/by_form_type/location/record_and_contents_subrecord_not_equally_based.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::location;
   }
}

#include "../utils/update_location_content.h"

namespace dovah::loaded_forms {
   void Location::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      bool is_base_record = intfc.current_file == intfc.target_stub.get_file_at_index(0);
      bool is_active_file = intfc.is_active_file();
      //
      form_reference_t formID;
      uint32_t  keywordCount = 0;
      while (auto& subrecord = record.next_subrecord()) {
         const auto signature = subrecord.signature();
         if (Form::subrecord_is_handled_elsewhere(signature))
            continue;

         // *CEC subrecord helpers
         auto cec_get_or_emplace_world = [](auto& worldmap, form_stub* worldspace) -> auto& {
            for (auto& entry : worldmap)
               if (entry.worldspace == worldspace)
                  return entry;
            auto& entry = worldmap.emplace_back();
            entry.worldspace.unmanaged_set(worldspace);
            return entry;
         };
         auto cec_insert_cell = [](auto& cellmap, const grid_coords& grid) {
            for (auto& cell : cellmap.cells)
               if (cell == grid)
                  return;
            cellmap.cells.push_back(grid);
         };
         auto cec_remove_cell = [](auto& cellmap, const grid_coords& grid) {
            std::erase(cellmap.cells, grid);
         };

         switch (signature) {
            #pragma region Content (*C**)
               #pragma region Enable Parents (*CEP)
                  case 'LCEP':
                  case 'ACEP':
                     {
                        bool intended_for_base = (subrecord.signature() >> 24) == 'L';
                        if (intended_for_base != is_base_record) {
                           specific_load_warnings::record_and_contents_subrecord_not_equally_based notice(
                              this->stub,
                              specific_load_warnings::record_and_contents_subrecord_not_equally_based::contents_type::enable_parent,
                              is_base_record
                           );
                           intfc.log_load_warning(notice);
                        }
                        auto& group = this->contents.enable_parents;
                        auto& item  = group.full.emplace_back();
                        subrecord.read(item.ref);
                        subrecord.read(item.enable_parent);
                        subrecord.read(item.flags);
                        if (is_base_record) {
                           auto& base_item = group.base.emplace_back();
                           base_item.ref.unmanaged_set(item.ref.get_form_stub());
                           base_item.enable_parent.unmanaged_set(item.enable_parent.get_form_stub());
                           base_item.flags = item.flags;
                        }
                     }
                     break;
               #pragma 
               #pragma region Exterior Cells (*CEC)
                  case 'LCEC':
                  case 'ACEC':
                     {
                        bool intended_for_base = (subrecord.signature() >> 24) == 'L';
                        if (intended_for_base != is_base_record) {
                           specific_load_warnings::record_and_contents_subrecord_not_equally_based notice(
                              this->stub,
                              specific_load_warnings::record_and_contents_subrecord_not_equally_based::contents_type::exterior_cell_list,
                              is_base_record
                           );
                           intfc.log_load_warning(notice);
                        }

                        struct {
                           form_reference_t worldspace;
                           std::vector<grid_coords> grid;
                        } src_item;
                        subrecord.read(src_item.worldspace);
                        while (subrecord.is_in_bounds(4)) {
                           auto& cell = src_item.grid.emplace_back();
                           subrecord.read(cell.y);
                           subrecord.read(cell.x);
                        }

                        auto& group = this->contents.exterior_cells;
                        if (is_base_record) {
                           auto& base_cellmap = cec_get_or_emplace_world(group.base,      src_item.worldspace.get_form_stub());
                           auto& adds_cellmap = cec_get_or_emplace_world(group.full, src_item.worldspace.get_form_stub());
                           for (auto& cell : src_item.grid) {
                              cec_insert_cell(base_cellmap, cell);
                              cec_insert_cell(adds_cellmap, cell);
                           }
                        } else {
                           auto& adds_cellmap = cec_get_or_emplace_world(group.full, src_item.worldspace.get_form_stub());
                           for (auto& cell : src_item.grid) {
                              cec_insert_cell(adds_cellmap, cell);
                           }
                        }
                     }
                     break;
                  case 'RCEC':
                     {
                        if (is_base_record) {
                           specific_load_warnings::base_record_should_not_have_content_removals notice(
                              this->stub,
                              specific_load_warnings::base_record_should_not_have_content_removals::contents_type::unique_actor
                           );
                           intfc.log_load_warning(notice);
                        }

                        struct {
                           form_reference_t worldspace;
                           std::vector<grid_coords> grid;
                        } src_item;
                        subrecord.read(src_item.worldspace);
                        while (subrecord.is_in_bounds(4)) {
                           auto& cell = src_item.grid.emplace_back();
                           subrecord.read(cell.y);
                           subrecord.read(cell.x);
                        }

                        auto& group = this->contents.exterior_cells;
                        if (is_base_record) {
                           auto& base_cellmap = cec_get_or_emplace_world(group.base, src_item.worldspace.get_form_stub());
                           for (auto& cell : src_item.grid) {
                              cec_remove_cell(base_cellmap, cell);
                           }
                        } else {
                           auto& adds_cellmap = cec_get_or_emplace_world(group.full, src_item.worldspace.get_form_stub());
                           for (auto& cell : src_item.grid) {
                              cec_remove_cell(adds_cellmap, cell);
                           }
                        }
                     }
                     break;
               #pragma endregion
               #pragma region Initially Disabled refs (*CID)
                  case 'LCID':
                  case 'ACID':
                     {
                        bool intended_for_base = (subrecord.signature() >> 24) == 'L';
                        if (intended_for_base != is_base_record) {
                           specific_load_warnings::record_and_contents_subrecord_not_equally_based notice(
                              this->stub,
                              specific_load_warnings::record_and_contents_subrecord_not_equally_based::contents_type::initially_disabled_ref,
                              is_base_record
                           );
                           intfc.log_load_warning(notice);
                        }
                        auto& group = this->contents.initially_disabled;
                        auto& item  = group.full.emplace_back();
                        subrecord.read(item);
                        if (is_base_record) {
                           auto& base_item = group.base.emplace_back();
                           base_item.unmanaged_set(item.get_form_stub());
                        }
                     }
                     break;
               #pragma endregion
               #pragma region UNique actors (*CUN)
                  case 'LCUN':
                  case 'ACUN':
                     {
                        bool intended_for_base = (subrecord.signature() >> 24) == 'L';
                        if (intended_for_base != is_base_record) {
                           specific_load_warnings::record_and_contents_subrecord_not_equally_based notice(
                              this->stub,
                              specific_load_warnings::record_and_contents_subrecord_not_equally_based::contents_type::unique_actor,
                              is_base_record
                           );
                           intfc.log_load_warning(notice);
                        }
                        auto& group = this->contents.unique_actors;
                        auto& item  = group.full.emplace_back();
                        subrecord.read(item.actor_base);
                        subrecord.read(item.actor);
                        subrecord.read(item.editor_location);
                        if (is_base_record) {
                           auto& base_item = group.base.emplace_back();
                           base_item.actor_base.unmanaged_set(item.actor_base.get_form_stub());
                           base_item.actor.unmanaged_set(item.actor.get_form_stub());
                           base_item.editor_location.unmanaged_set(item.editor_location.get_form_stub());
                        }
                     }
                     break;
                  case 'RCUN':
                     {
                        form_id_t id;
                        subrecord.read(id);
                        if (!id)
                           break;

                        if (is_base_record) {
                           specific_load_warnings::base_record_should_not_have_content_removals notice(
                              this->stub,
                              specific_load_warnings::base_record_should_not_have_content_removals::contents_type::unique_actor
                           );
                           intfc.log_load_warning(notice);
                        }
                        auto& group = this->contents.unique_actors;
                        auto& list  = is_base_record ? group.base : group.full;
                        std::erase_if(list, [id](auto& item) {
                           auto* stub = item.actor_base.get_form_stub();
                           return stub && stub->formID == id;
                        });
                     }
                     break;
               #pragma endregion
               #pragma region Persist location Refs (*CPR)
                  case 'LCPR':
                  case 'ACPR':
                     {
                        bool intended_for_base = (subrecord.signature() >> 24) == 'L';
                        if (intended_for_base != is_base_record) {
                           specific_load_warnings::record_and_contents_subrecord_not_equally_based notice(
                              this->stub,
                              specific_load_warnings::record_and_contents_subrecord_not_equally_based::contents_type::persist_location_ref,
                              is_base_record
                           );
                           intfc.log_load_warning(notice);
                        }
                        auto& group = this->contents.persistent_refs;
                        auto& item  = group.full.emplace_back();
                        subrecord.read(item.ref);
                        subrecord.read(item.cell_or_world);
                        subrecord.read(item.grid.y);
                        subrecord.read(item.grid.x);
                        if (is_base_record) {
                           auto& base_item = group.base.emplace_back();
                           base_item.ref.unmanaged_set(item.ref.get_form_stub());
                           base_item.cell_or_world.unmanaged_set(item.cell_or_world.get_form_stub());
                           base_item.grid = item.grid;
                        }
                     }
                     break;
                  case 'RCPR':
                     {
                        form_id_t id;
                        subrecord.read(id);
                        if (!id)
                           break;

                        if (is_base_record) {
                           specific_load_warnings::base_record_should_not_have_content_removals notice(
                              this->stub,
                              specific_load_warnings::base_record_should_not_have_content_removals::contents_type::persist_location_ref
                           );
                           intfc.log_load_warning(notice);
                        }
                        auto& group = this->contents.persistent_refs;
                        auto& list  = is_base_record ? group.base : group.full;
                        std::erase_if(list, [id](auto& item) {
                           auto* stub = item.ref.get_form_stub();
                           return stub && stub->formID == id;
                        });
                     }
                     break;
               #pragma endregion
               #pragma region Special Refs (*CSR)
                  case 'LCSR':
                  case 'ACSR':
                     {
                        bool intended_for_base = (subrecord.signature() >> 24) == 'L';
                        if (intended_for_base != is_base_record) {
                           specific_load_warnings::record_and_contents_subrecord_not_equally_based notice(
                              this->stub,
                              specific_load_warnings::record_and_contents_subrecord_not_equally_based::contents_type::special_ref,
                              is_base_record
                           );
                           intfc.log_load_warning(notice);
                        }
                        auto& group = this->contents.special_refs;
                        auto& item = group.full.emplace_back();
                        subrecord.read(item.ref_type);
                        subrecord.read(item.reference);
                        subrecord.read(item.cell_or_world);
                        subrecord.read(item.grid.y);
                        subrecord.read(item.grid.x);
                        if (is_base_record) {
                           auto& base_item = group.base.emplace_back();
                           base_item.ref_type.unmanaged_set(item.ref_type.get_form_stub());
                           base_item.reference.unmanaged_set(item.reference.get_form_stub());
                           base_item.cell_or_world.unmanaged_set(item.cell_or_world.get_form_stub());
                           base_item.grid = item.grid;
                        }
                     }
                     break;
                  case 'RCSR':
                     {
                        form_id_t id;
                        subrecord.read(id);
                        if (!id)
                           break;

                        if (is_base_record) {
                           specific_load_warnings::base_record_should_not_have_content_removals notice(
                              this->stub,
                              specific_load_warnings::base_record_should_not_have_content_removals::contents_type::special_ref
                           );
                           intfc.log_load_warning(notice);
                        }
                        auto& group = this->contents.special_refs;
                        auto& list = is_base_record ? group.base : group.full;
                        std::erase_if(list, [id](auto& item) {
                           auto* stub = item.reference.get_form_stub();
                           return stub && stub->formID == id;
                        });
                     }
                     break;
               #pragma endregion
            #pragma endregion

            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               if (!intfc.is_winning_record)
                  break;
               this->keywords.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'PNAM':
               if (!intfc.is_winning_record)
                  break;
               if (auto& form = this->parent_location; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::location, subrecord.signature());
               break;
            case 'NAM1':
               if (!intfc.is_winning_record)
                  break;
               if (auto& form = this->music; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::music_type, subrecord.signature());
               break;
            case 'FNAM':
               if (!intfc.is_winning_record)
                  break;
               if (auto& form = this->unreported_crime_faction; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::faction, subrecord.signature());
               break;
            case 'MNAM':
               if (!intfc.is_winning_record)
                  break;
               if (auto& form = this->marker; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::reference, subrecord.signature());
               break;
            case 'RNAM':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->radius);
               break;
            case 'NAM0':
               if (!intfc.is_winning_record)
                  break;
               if (auto& form = this->horse_marker; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::reference, subrecord.signature());
               break;
            case 'CNAM':
               if (!intfc.is_winning_record)
                  break;
               this->color.load(subrecord);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Location::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      const bool is_base_record = uib.is_base_record();

      struct enable_parentage {
         form_id_t ref;
         form_id_t enable_parent;
      };
      struct persist_loc_ref {
         form_id_t ref;
         form_id_t cell_or_world;
      };
      struct special_ref {
         form_id_t ref_type;
         form_id_t reference;
         form_id_t cell_or_world;
      };
      struct unique_actor {
         form_id_t actor_base;
         form_id_t actor;
         form_id_t editor_location;
      };

      struct {
         content_list<enable_parentage> enable_parentages;
         content_list<form_id_t>        exterior_cell_parents;
         content_list<form_id_t>        initially_disabled;
         content_list<persist_loc_ref>  persist_loc_refs;
         content_list<special_ref>      special_refs;
         content_list<unique_actor>     unique_actors;
      } contents;
      form_id_t crime_faction;
      form_id_t marker;
      form_id_t horse_marker;
      form_id_t music;
      form_id_t parent;

      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               if (!uib.is_final_file())
                  break;
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               if (!uib.is_final_file())
                  break;
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            //
            #pragma region Contents
               #pragma region Enable parents
                  case 'LCEP':
                  case 'ACEP':
                     {
                        auto& dataset = contents.enable_parentages;
                        auto& item = dataset.full.emplace_back();
                        subrecord.read(item.ref);
                        subrecord.read(item.enable_parent);
                        if (is_base_record) {
                           dataset.base.push_back(item);
                        }
                     }
                     break;
               #pragma endregion
               #pragma region Exterior Cells
                  case 'LCEC':
                  case 'ACEC':
                     {
                        auto& dataset = contents.initially_disabled;
                        auto& item = dataset.full.emplace_back();
                        subrecord.read(item);
                        if (is_base_record) {
                           dataset.base.push_back(item);
                        }
                     }
                     break;
                  case 'RCEC':
                     {
                        //
                        // Currently, the loader doesn't trim out worldspaces that are defined via 
                        // ACEC and become empty via RCEC.
                     }
                     break;
               #pragma endregion
               #pragma region Initially Disabled
                  case 'LCID':
                  case 'ACID':
                     {
                        auto& dataset = contents.initially_disabled;
                        auto& item = dataset.full.emplace_back();
                        subrecord.read(item);
                        if (is_base_record) {
                           dataset.base.push_back(item);
                        }
                     }
                     break;
               #pragma endregion
               #pragma region Persist loc Refs
                  case 'LCPR':
                  case 'ACPR':
                     {
                        auto& dataset = contents.persist_loc_refs;
                        auto& item = dataset.full.emplace_back();
                        subrecord.read(item.ref);
                        subrecord.read(item.cell_or_world);
                        if (is_base_record) {
                           dataset.base.push_back(item);
                        }
                     }
                     break;
                  case 'RCPR':
                     {
                        form_id_t ref;
                        subrecord.read(ref);

                        auto&  dataset = contents.persist_loc_refs;
                        auto&  list    = dataset.full;
                        size_t size    = list.size();
                        for (size_t i = 0; i < size; ++i) {
                           if (list[i].ref == ref) {
                              list.erase(list.begin() + i);
                              --i;
                              --size;
                           }
                        }
                     }
                     break;
               #pragma endregion
               #pragma region Special Refs
                  case 'LCSR':
                  case 'ACSR':
                     {
                        auto& dataset = contents.special_refs;
                        auto& item = dataset.full.emplace_back();
                        subrecord.read(item.ref_type);
                        subrecord.read(item.reference);
                        subrecord.read(item.cell_or_world);
                        if (is_base_record) {
                           dataset.base.push_back(item);
                        }
                     }
                     break;
                  case 'RCSR':
                     {
                        form_id_t ref;
                        subrecord.read(ref);

                        auto&  dataset = contents.special_refs;
                        auto&  list    = dataset.full;
                        size_t size    = list.size();
                        for (size_t i = 0; i < size; ++i) {
                           if (list[i].reference == ref) {
                              list.erase(list.begin() + i);
                              --i;
                              --size;
                           }
                        }
                     }
                     break;
               #pragma endregion
               #pragma region UNique actors
                  case 'LCUN':
                  case 'ACUN':
                     {
                        auto& dataset = contents.unique_actors;
                        auto& item = dataset.full.emplace_back();
                        subrecord.read(item.actor_base);
                        subrecord.read(item.actor);
                        subrecord.read(item.editor_location);
                        if (is_base_record) {
                           dataset.base.push_back(item);
                        }
                     }
                     break;
                  case 'RCUN':
                     {
                        form_id_t actor_base;
                        subrecord.read(actor_base);

                        auto&  dataset = contents.unique_actors;
                        auto&  list    = dataset.full;
                        size_t size    = list.size();
                        for (size_t i = 0; i < size; ++i) {
                           if (list[i].actor_base == actor_base) {
                              list.erase(list.begin() + i);
                              --i;
                              --size;
                           }
                        }
                     }
                     break;
               #pragma endregion
            #pragma endregion
            //
            case 'PNAM': // parent location
               subrecord.read(parent);
               break;
            case 'NAM1': // music
               subrecord.read(music);
               break;
            case 'FNAM': // unreported crime faction
               subrecord.read(crime_faction);
               break;
            case 'MNAM': // marker
               subrecord.read(marker);
               break;
            case 'NAM0': // horse marker
               subrecord.read(horse_marker);
               break;
            case 'EDID': // editor ID
            case 'FULL': // name
            case 'RNAM': // radius
            case 'CNAM': // color
               break;
         }
      }
      //
      // Commit:
      //
      #pragma region Contents
         #pragma region Enable parents
            {
               auto& dataset = contents.enable_parentages;
               auto  commit  = [&uib](auto& list) {
                  for (auto& item : list) {
                     uib.add_outbound_reference(item.ref);
                     uib.add_outbound_reference(item.enable_parent);
                  }
               };
               commit(dataset.base);
               commit(dataset.full);
            }
         #pragma endregion
         #pragma region Exterior Cells
            {
               auto& dataset = contents.initially_disabled;
               auto  commit  = [&uib](auto& list) {
                  for (auto& item : list) {
                     uib.add_outbound_reference(item);
                  }
               };
               commit(dataset.base);
               commit(dataset.full);
            }
         #pragma endregion
         #pragma region Initially Disabled
            {
               auto& dataset = contents.initially_disabled;
               auto  commit  = [&uib](auto& list) {
                  for (auto& item : list)
                     uib.add_outbound_reference(item);
               };
               commit(dataset.base);
               commit(dataset.full);
            }
         #pragma endregion
         #pragma region Persist loc Refs
            {
               auto& dataset = contents.persist_loc_refs;
               auto  commit  = [&uib](auto& list) {
                  for (auto& item : list) {
                     uib.add_outbound_reference(item.ref);
                     uib.add_outbound_reference(item.cell_or_world);
                  }
               };
               commit(dataset.base);
               commit(dataset.full);
            }
         #pragma endregion
         #pragma region Special Refs
            {
               auto& dataset = contents.special_refs;
               auto  commit  = [&uib](auto& list) {
                  for (auto& item : list) {
                     uib.add_outbound_reference(item.ref_type);
                     uib.add_outbound_reference(item.reference);
                     uib.add_outbound_reference(item.cell_or_world);
                  }
               };
               commit(dataset.base);
               commit(dataset.full);
            }
         #pragma endregion
         #pragma region UNique actors
            {
               auto& dataset = contents.unique_actors;
               auto  commit  = [&uib](auto& list) {
                  for (auto& item : list) {
                     uib.add_outbound_reference(item.actor_base);
                     uib.add_outbound_reference(item.actor);
                     uib.add_outbound_reference(item.editor_location);
                  }
               };
               commit(dataset.base);
               commit(dataset.full);
            }
         #pragma endregion
      #pragma endregion
      if (uib.is_final_file()) { // These fields are not coalesced across files.
         uib.add_outbound_reference(crime_faction);
         uib.add_outbound_reference(marker);
         uib.add_outbound_reference(horse_marker);
         uib.add_outbound_reference(music);
         uib.add_outbound_reference(parent);
      }
   }
   void Location::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Location*)out;
      
      copy->clear();

      copy->keywords.clone_from(this->keywords, *copy);
      copy->script_data.clone_from(this->script_data, *copy);
      
      #pragma region Contents
         {
            auto& src_dataset = this->contents.enable_parents;
            auto& dst_dataset = copy->contents.enable_parents;
            auto  clone   = [this, copy](const auto& src_list, auto& dst_list) {
               size_t size = src_list.size();
               dst_list.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  dst_list[i].ref.set(*copy, src_list[i].ref);
                  dst_list[i].enable_parent.set(*copy, src_list[i].enable_parent);
                  dst_list[i].flags = src_list[i].flags;
               }
            };
            clone(src_dataset.base,      dst_dataset.base);
            clone(src_dataset.full, dst_dataset.full);
         }
         {
            auto& src_dataset = this->contents.exterior_cells;
            auto& dst_dataset = copy->contents.exterior_cells;
            auto  clone   = [this, copy](const auto& src_list, auto& dst_list) {
               size_t size = src_list.size();
               dst_list.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  dst_list[i].worldspace.set(*copy, src_list[i].worldspace);
                  dst_list[i].cells = src_list[i].cells;
               }
            };
            clone(src_dataset.base,      dst_dataset.base);
            clone(src_dataset.full, dst_dataset.full);
         }
         {
            auto& src_dataset = this->contents.initially_disabled;
            auto& dst_dataset = copy->contents.initially_disabled;
            copy_form_reference_list(*copy, dst_dataset.base,      src_dataset.base);
            copy_form_reference_list(*copy, dst_dataset.full, src_dataset.full);
         }
         {
            auto& src_dataset = this->contents.persistent_refs;
            auto& dst_dataset = copy->contents.persistent_refs;
            auto  clone   = [this, copy](const auto& src_list, auto& dst_list) {
               size_t size = src_list.size();
               dst_list.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  dst_list[i].ref.set(*copy, src_list[i].ref);
                  dst_list[i].cell_or_world = src_list[i].cell_or_world;
                  dst_list[i].grid = src_list[i].grid;
               }
            };
            clone(src_dataset.base,      dst_dataset.base);
            clone(src_dataset.full, dst_dataset.full);
         }
         {
            auto& src_dataset = this->contents.special_refs;
            auto& dst_dataset = copy->contents.special_refs;
            auto  clone   = [this, copy](const auto& src_list, auto& dst_list) {
               size_t size = src_list.size();
               dst_list.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  dst_list[i].ref_type.set(*copy, src_list[i].ref_type);
                  dst_list[i].reference.set(*copy, src_list[i].reference);
                  dst_list[i].cell_or_world = src_list[i].cell_or_world;
                  dst_list[i].grid = src_list[i].grid;
               }
            };
            clone(src_dataset.base,      dst_dataset.base);
            clone(src_dataset.full, dst_dataset.full);
         }
         {
            auto& src_dataset = this->contents.unique_actors;
            auto& dst_dataset = copy->contents.unique_actors;
            auto  clone   = [this, copy](const auto& src_list, auto& dst_list) {
               size_t size = src_list.size();
               dst_list.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  dst_list[i].actor.set(*copy, src_list[i].actor);
                  dst_list[i].actor_base.set(*copy, src_list[i].actor_base);
                  dst_list[i].editor_location.set(*copy, src_list[i].editor_location);
               }
            };
            clone(src_dataset.base,      dst_dataset.base);
            clone(src_dataset.full, dst_dataset.full);
         }
      #pragma endregion

      copy->name = this->name;
      copy->parent_location.set(*copy, this->parent_location);
      copy->music.set(*copy, this->music);
      copy->unreported_crime_faction.set(*copy, this->unreported_crime_faction);
      copy->marker.set(*copy, this->marker);
      copy->radius = this->radius;
      copy->horse_marker.set(*copy, this->horse_marker);
      copy->color = this->color;
   }
   void Location::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      const bool is_base_record = this->stub.get_owning_load_order().is_defined_in_active_file(this->stub);
      {
         utils::update_location_content updater;
         updater.gather(this->stub);
         updater.apply(is_base_record);
      }

      this->script_data.save(record, intfc);

      #pragma region Contents
         #pragma region *CPR
            {
               const auto& dataset = this->contents.persistent_refs;
               if (is_base_record) {
                  auto& subrecord = record.open_next_subrecord('LCPR');
                  for (const auto& item : dataset.base) {
                     subrecord.write(item.ref);
                     subrecord.write(item.cell_or_world);
                     subrecord.write(item.grid.y);
                     subrecord.write(item.grid.x);
                  }
                  subrecord.close();
               } else {
                  std::vector<form_stub*> retained;
                  bool opened = false;
                  //
                  // ACPR:
                  //
                  for (const auto& item : dataset.full) {
                     retained.push_back(item.ref.get_form_stub());
                     //
                     bool unchanged = false;
                     for (const auto& prior : dataset.base) {
                        if (prior.ref == item.ref) {
                           unchanged = item == prior;
                           break;
                        }
                     }
                     if (!unchanged) {
                        if (!opened) {
                           record.open_next_subrecord('ACPR');
                           opened = true;
                        }
                        auto& subrecord = record.get_current_subrecord();
                        subrecord.write(item.ref);
                        subrecord.write(item.cell_or_world);
                        subrecord.write(item.grid.y);
                        subrecord.write(item.grid.x);
                     }
                  }
                  if (opened) {
                     record.get_current_subrecord().close();
                  }
                  //
                  // RCPR:
                  //
                  opened = false;
                  for (const auto& item : dataset.base) {
                     auto it = std::find(
                        retained.begin(),
                        retained.end(),
                        item.ref.get_form_stub()
                     );
                     if (it == retained.end()) {
                        if (!opened) {
                           record.open_next_subrecord('RCPR');
                           opened = true;
                        }
                        auto& subrecord = record.get_current_subrecord();
                        subrecord.write(item.ref);
                     }
                  }
                  if (opened) {
                     record.get_current_subrecord().close();
                  }
               }
            }
         #pragma endregion
         #pragma region *CUN
            {
               const auto& dataset = this->contents.unique_actors;
               if (is_base_record) {
                  auto& subrecord = record.open_next_subrecord('LCUN');
                  for (const auto& item : dataset.base) {
                     subrecord.write(item.actor_base);
                     subrecord.write(item.actor);
                     subrecord.write(item.editor_location);
                  }
                  subrecord.close();
               } else {
                  std::vector<form_stub*> retained;
                  bool opened = false;
                  //
                  // Added:
                  //
                  for (const auto& item : dataset.full) {
                     retained.push_back(item.actor_base.get_form_stub());
                     //
                     bool unchanged = false;
                     for (const auto& prior : dataset.base) {
                        if (prior.actor_base == item.actor_base) {
                           unchanged = item == prior;
                           break;
                        }
                     }
                     if (!unchanged) {
                        if (!opened) {
                           record.open_next_subrecord('ACUN');
                           opened = true;
                        }
                        auto& subrecord = record.get_current_subrecord();
                        subrecord.write(item.actor_base);
                        subrecord.write(item.actor);
                        subrecord.write(item.editor_location);
                     }
                  }
                  if (opened) {
                     record.get_current_subrecord().close();
                  }
                  //
                  // Removed:
                  //
                  opened = false;
                  for (const auto& item : dataset.base) {
                     auto it = std::find(
                        retained.begin(),
                        retained.end(),
                        item.actor_base.get_form_stub()
                     );
                     if (it == retained.end()) {
                        if (!opened) {
                           record.open_next_subrecord('RCUN');
                           opened = true;
                        }
                        auto& subrecord = record.get_current_subrecord();
                        subrecord.write(item.actor_base);
                     }
                  }
                  if (opened) {
                     record.get_current_subrecord().close();
                  }
               }
            }
         #pragma endregion
         #pragma region *CSR
            {
               const auto& dataset = this->contents.special_refs;
               if (is_base_record) {
                  auto& subrecord = record.open_next_subrecord('LCSR');
                  for (const auto& item : dataset.base) {
                     subrecord.write(item.ref_type);
                     subrecord.write(item.reference);
                     subrecord.write(item.cell_or_world);
                     subrecord.write(item.grid.y);
                     subrecord.write(item.grid.x);
                  }
                  subrecord.close();
               } else {
                  std::vector<form_stub*> retained;
                  bool opened = false;
                  //
                  // Added:
                  //
                  for (const auto& item : dataset.full) {
                     retained.push_back(item.reference.get_form_stub());
                     //
                     bool unchanged = false;
                     for (const auto& prior : dataset.base) {
                        if (prior.reference == item.reference) {
                           unchanged = item == prior;
                           break;
                        }
                     }
                     if (!unchanged) {
                        if (!opened) {
                           record.open_next_subrecord('ACSR');
                           opened = true;
                        }
                        auto& subrecord = record.get_current_subrecord();
                        subrecord.write(item.ref_type);
                        subrecord.write(item.reference);
                        subrecord.write(item.cell_or_world);
                        subrecord.write(item.grid.y);
                        subrecord.write(item.grid.x);
                     }
                  }
                  if (opened) {
                     record.get_current_subrecord().close();
                  }
                  //
                  // Removed:
                  //
                  opened = false;
                  for (const auto& item : dataset.base) {
                     auto it = std::find(
                        retained.begin(),
                        retained.end(),
                        item.reference.get_form_stub()
                     );
                     if (it == retained.end()) {
                        if (!opened) {
                           record.open_next_subrecord('RCSR');
                           opened = true;
                        }
                        auto& subrecord = record.get_current_subrecord();
                        subrecord.write(item.reference);
                     }
                  }
                  if (opened) {
                     record.get_current_subrecord().close();
                  }
               }
            }
         #pragma endregion
         #pragma region *CEC
            {
               const auto& dataset = this->contents.exterior_cells;
               if (is_base_record) {
                  for (const auto& item : dataset.base) {
                     auto& subrecord = record.open_next_subrecord('LCEC');
                     subrecord.write(item.worldspace);
                     for (auto& cell : item.cells) {
                        subrecord.write(cell.y);
                        subrecord.write(cell.x);
                     }
                     subrecord.close();
                  }
               } else {
                  struct relevant_cell {
                     grid_coords grid;
                     bool        retained = false;
                  };
                  using relevant_world_map = std::unordered_map<form_stub*, std::vector<relevant_cell>>;
                  relevant_world_map relevant_worlds;

                  auto _gather = [&relevant_worlds](const exterior_cell_list& ecl, bool retained) {
                     auto& rcl = relevant_worlds[ecl.worldspace.get_form_stub()];
                     for (auto& src_item : ecl.cells) {
                        bool found = false;
                        for (auto& dst_item : rcl) {
                           if (src_item == dst_item.grid) {
                              dst_item.retained = retained;
                              found = true;
                              break;
                           }
                        }
                        if (!found) {
                           auto& dst_item = rcl.emplace_back();
                           dst_item.grid     = src_item;
                           dst_item.retained = retained;
                        }
                     }
                  };
                  for (const auto& item : dataset.base)
                     _gather(item, false);
                  for (const auto& item : dataset.full)
                     _gather(item, true);
                  //
                  // Added:
                  //
                  bool any_not_retained = false;
                  for (auto& pair : relevant_worlds) {
                     bool opened = false;
                     for (auto& cell : pair.second) {
                        if (!cell.retained) {
                           any_not_retained = true;
                           continue;
                        }
                        if (!opened) {
                           opened = true;
                           auto& subrecord = record.open_next_subrecord('ACEC');
                           subrecord.write(pair.first);
                        }
                        auto& subrecord = record.get_current_subrecord();
                        subrecord.write(cell.grid.y);
                        subrecord.write(cell.grid.x);
                     }
                     if (opened) {
                        record.get_current_subrecord().close();
                     }
                  }
                  if (any_not_retained) {
                     //
                     // Removed:
                     //
                     for (auto& pair : relevant_worlds) {
                        bool opened = false;
                        for (auto& cell : pair.second) {
                           if (cell.retained) {
                              continue;
                           }
                           if (!opened) {
                              opened = true;
                              auto& subrecord = record.open_next_subrecord('RCEC');
                              subrecord.write(pair.first);
                           }
                           auto& subrecord = record.get_current_subrecord();
                           subrecord.write(cell.grid.y);
                           subrecord.write(cell.grid.x);
                        }
                        if (opened) {
                           record.get_current_subrecord().close();
                        }
                     }
                  }
               }
            }
         #pragma endregion
         #pragma region *CID
            {
               const auto& dataset = this->contents.initially_disabled;
               const auto& list    = is_base_record ? dataset.base : dataset.full;

               auto& subrecord = record.open_next_subrecord(is_base_record ? 'LCID' : 'ACID');
               for (const auto& item : list) {
                  subrecord.write(item);
               }
               subrecord.close();
            }
         #pragma endregion
         #pragma region *CEP
            {
               const auto& dataset = this->contents.enable_parents;
               const auto& list    = is_base_record ? dataset.base : dataset.full;

               auto& subrecord = record.open_next_subrecord(is_base_record ? 'LCEP' : 'ACEP');
               for (const auto& item : list) {
                  subrecord.write(item.ref);
                  subrecord.write(item.enable_parent);
                  subrecord.write(item.flags);
               }
               subrecord.close();
            }
         #pragma endregion
      #pragma endregion
            
      if (!this->name.empty()) {
         auto& FULL = record.open_next_subrecord('FULL');
         FULL.write(this->name);
         FULL.close();
      }
      record.write_formID_subrecord('PNAM', this->parent_location, true);
      record.write_formID_subrecord('NAM1', this->music, true);
      record.write_formID_subrecord('FNAM', this->unreported_crime_faction, true);
      record.write_formID_subrecord('MNAM', this->marker, true);
      {
         auto& subrecord = record.open_next_subrecord('RNAM');
         subrecord.write(this->radius);
         subrecord.close();
      }
      record.write_formID_subrecord('NAM0', this->horse_marker, true);
      {
         auto& subrecord = record.open_next_subrecord('CNAM');
         this->color.save(subrecord);
         subrecord.close();
      }
   }
   void Location::_clear_impl() noexcept {
      this->keywords.clear(*this);
      this->script_data.clear(*this);

      #pragma region Contents
         {
            auto& dataset = this->contents.enable_parents;
            auto  clear   = [this](decltype(std::decay_t<decltype(dataset)>::base)& list) {
               for (auto& item : list) {
                  item.ref.set(*this, nullptr);
                  item.enable_parent.set(*this, nullptr);
               }
               list.clear();
            };
            clear(dataset.base);
            clear(dataset.full);
         }
         {
            auto& dataset = this->contents.exterior_cells;
            auto  clear   = [this](decltype(std::decay_t<decltype(dataset)>::base)& list) {
               for (auto& item : list) {
                  item.worldspace.set(*this, nullptr);
               }
               list.clear();
            };
            clear(dataset.base);
            clear(dataset.full);
         }
         {
            auto& dataset = this->contents.initially_disabled;
            clear_form_reference_list(dataset.base, *this);
            clear_form_reference_list(dataset.full, *this);
         }
         {
            auto& dataset = this->contents.persistent_refs;
            auto  clear   = [this](decltype(std::decay_t<decltype(dataset)>::base)& list) {
               for (auto& item : list) {
                  item.ref.set(*this, nullptr);
                  item.cell_or_world.set(*this, nullptr);
               }
               list.clear();
            };
            clear(dataset.base);
            clear(dataset.full);
         }
         {
            auto& dataset = this->contents.special_refs;
            auto  clear   = [this](decltype(std::decay_t<decltype(dataset)>::base)& list) {
               for (auto& item : list) {
                  item.ref_type.set(*this, nullptr);
                  item.reference.set(*this, nullptr);
                  item.cell_or_world.set(*this, nullptr);
               }
               list.clear();
            };
            clear(dataset.base);
            clear(dataset.full);
         }
         {
            auto& dataset = this->contents.unique_actors;
            auto  clear   = [this](decltype(std::decay_t<decltype(dataset)>::base)& list) {
               for (auto& item : list) {
                  item.actor_base.set(*this, nullptr);
                  item.actor.set(*this, nullptr);
                  item.editor_location.set(*this, nullptr);
               }
               list.clear();
            };
            clear(dataset.base);
            clear(dataset.full);
         }
      #pragma endregion

      this->name.reset();
      this->parent_location.set(*this, nullptr);
      this->music.set(*this, nullptr);
      this->unreported_crime_faction.set(*this, nullptr);
      this->marker.set(*this, nullptr);
      this->horse_marker.set(*this, nullptr);
      this->color = {};
   }
   void Location::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->keywords.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      
      #pragma region Contents
         {
            auto& dataset = this->contents.enable_parents;
            auto  sever   = [this, &other](decltype(std::decay_t<decltype(dataset)>::base)& list) {
               bool any_removed = false;
               for (auto& item : list) {
                  item.ref.clear_if(*this, other);
                  item.enable_parent.clear_if(*this, other);
                  if (!item.ref || !item.enable_parent) {
                     any_removed = true;
                  }
               }
               if (any_removed) {
                  std::erase_if(list, [](const auto& item) -> bool {
                     return !item.ref;
                  });
               }
            };
            sever(dataset.base);
            sever(dataset.full);
         }
         {
            auto& dataset = this->contents.exterior_cells;
            auto  sever   = [this, &other](decltype(std::decay_t<decltype(dataset)>::base)& list) {
               if (other.form_type == form_type::worldspace) {
                  bool any_removed = false;
                  for (auto& item : list) {
                     item.worldspace.clear_if(*this, other);
                     if (!item.worldspace)
                        any_removed = true;
                  }
                  if (any_removed)
                     std::erase_if(list, [](const auto& item) -> bool { return !item.worldspace; });
                  return;
               }
               if (other.form_type == form_type::cell) {
                  int32_t x;
                  int32_t y;
                  if (!other.get_grid_coordinates(x, y))
                     return;
                  const auto* world = other.get_parent_form();
                  if (!world || world->form_type != form_type::worldspace)
                     return;
                  for (auto& item : list) {
                     if (item.worldspace != world)
                        continue;
                     size_t size = item.cells.size();
                     for (size_t i = 0; i < size; ++i) {
                        auto& cell = item.cells[i];
                        if (cell.x == x && cell.y == y) {
                           item.cells.erase(item.cells.begin() + i);
                           --i;
                           --size;
                        }
                     }
                  }
                  return;
               }
            };
            sever(dataset.base);
            sever(dataset.full);
         }
         {
            auto& dataset = this->contents.initially_disabled;
            remove_form_from_reference_list(dataset.base, other, *this);
            remove_form_from_reference_list(dataset.full, other, *this);
         }
         {
            auto& dataset = this->contents.persistent_refs;
            auto  sever   = [this, &other](decltype(std::decay_t<decltype(dataset)>::base)& list) {
               bool any_removed = false;
               for (auto& item : list) {
                  item.ref.clear_if(*this, other);
                  item.cell_or_world.clear_if(*this, other);
               }
               if (any_removed) {
                  std::erase_if(list, [](const auto& item) -> bool {
                     return !item.ref;
                  });
               }
            };
            sever(dataset.base);
            sever(dataset.full);
         }
         {
            auto& dataset = this->contents.special_refs;
            auto  sever   = [this, &other](decltype(std::decay_t<decltype(dataset)>::base)& list) {
               bool any_removed = false;
               for (auto& item : list) {
                  item.ref_type.clear_if(*this, other);
                  item.reference.clear_if(*this, other);
                  item.cell_or_world.clear_if(*this, other);
                  any_removed |= !item.reference;
               }
               if (any_removed) {
                  std::erase_if(list, [](const auto& item) -> bool {
                     return !item.reference;
                  });
               }
            };
            sever(dataset.base);
            sever(dataset.full);
         }
         {
            auto& dataset = this->contents.unique_actors;
            auto  sever   = [this, &other](decltype(std::decay_t<decltype(dataset)>::base)& list) {
               bool any_removed = false;
               for (auto& item : list) {
                  item.actor_base.clear_if(*this, other);
                  item.actor.clear_if(*this, other);
                  item.editor_location.clear_if(*this, other);
                  any_removed |= !item.actor;
               }
               if (any_removed) {
                  std::erase_if(list, [](const auto& item) -> bool {
                     return !item.actor;
                  });
               }
            };
            sever(dataset.base);
            sever(dataset.full);
         }
      #pragma endregion

      this->parent_location.clear_if(*this, other);
      this->music.clear_if(*this, other);
      this->unreported_crime_faction.clear_if(*this, other);
      this->marker.clear_if(*this, other);
      this->horse_marker.clear_if(*this, other);
   }
}