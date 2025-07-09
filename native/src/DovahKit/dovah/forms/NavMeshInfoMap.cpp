#include "NavMeshInfoMap.h"
#include "_common_cpp.h"
#include "../form_stub_use_info_builder_form_specific_data.h"

#include "../notices/form_load_warnings/by_form_type/navmesh_info_map/navmesh_may_be_multiply_deleted.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::navmesh_info_map;
   }
}

#include "../files/tes_file_reading/file_loader.h"
#include "../form_stubs/helpers/for_each_child_form.h"
#include "../form_stubs/helpers/get_worldspace_persistent_cell.h"
#include "../form_stubs/helpers/is_worldspace_persistent_cell.h"
namespace {
   static bool _is_defined_and_overridden_in_different_masters(dovah::form_stub& stub) {
      auto* defined_in    = stub.get_file_at_index(0);
      auto* overridden_in = stub.get_file_at_index(-1);
      if (defined_in == overridden_in)
         return false;
      assert(defined_in    != nullptr);
      assert(overridden_in != nullptr);
      if (!defined_in->is_master() || !overridden_in->is_master())
         return false;
      return true;
   }

   static std::vector<dovah::form_stub*> _find_all_deleted_navmeshes_of_interest(dovah::file_load_order& lo) {
      //
      // The CK manually crawls all interior cells, and then all worldspaces. However, we have the 
      // option to just scan all navmesh-type forms directly.
      //
      constexpr const bool use_direct_scan = true;

      std::vector<dovah::form_stub*> out;

      if constexpr (use_direct_scan) {
         lo.for_each_form_of_type(dovah::form_type::navmesh, [&out](dovah::form_stub* stub) -> bool {
            if (!stub->is_deleted())
               return false;
            if (!_is_defined_and_overridden_in_different_masters(*stub))
               return false;
            out.push_back(stub);
            return false;
         });
      } else {
         auto _scan_cell = [&out](dovah::form_stub& cell) {
            dovah::form_stub_helpers::for_each_child_form(&cell, [&out](dovah::form_stub* ref) {
               if (ref->form_type != dovah::form_type::navmesh)
                  return;
               if (!ref->is_deleted())
                  return;
               if (!_is_defined_and_overridden_in_different_masters(*ref))
                  return;
               out.push_back(ref);
            });
         };

         lo.for_each_form_of_type(dovah::form_type::cell, [&_scan_cell](dovah::form_stub* stub) -> bool {
            if (stub->is_exterior_cell())
               return false;
            _scan_cell(*stub);
            return false;
         });
         lo.for_each_form_of_type(dovah::form_type::worldspace, [&_scan_cell](dovah::form_stub* world) -> bool {
            auto* pcell = dovah::form_stub_helpers::get_worldspace_persistent_cell(world);
            if (pcell) {
               _scan_cell(*pcell);
            }
            dovah::form_stub_helpers::for_each_child_form(world, [&_scan_cell, pcell](dovah::form_stub* cell) {
               if (cell->form_type != dovah::form_type::cell)
                  return;
               if (cell == pcell)
                  return;
               _scan_cell(*cell);
            });
         });
      }

      return out;
   }
}

namespace dovah::loaded_forms {
   void NavMeshInfoMap::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      uint32_t version = 0; // NVER
      //
      bool is_active_file = intfc.is_active_file();
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'NVER':
               subrecord.read(version);
               break;
            case 'NVMI':
               //
               // In-game and in the CK, NVMI are coalesced across all NAVI records in all files. They 
               // are written into a map of navmesh pointers to loaded NVMI data, so overrides exist 
               // per-subrecord (if an NVMI subrecord attaches itself to a navmesh for which we have 
               // already seen an NVMI before).
               //
               this->navmesh_infos.load_one(subrecord, intfc);
               break;
            case 'NVPP':
               //
               // NVPP consists of two parts: a list of precomputed paths, and a list of road markers.
               // 
               // The precomputed path list is cleared every time we see a new NVPP subrecord, and then 
               // loaded from the first part of the subrecord. This list, then, obeys the Rule of One 
               // but on a per-subrecord basis only (i.e. if the winning NAVI override lacked NVPP, then 
               // NVPP would be taken from the latest "overridden" record which has one).
               // 
               // The road marker list is not cleared, but is loaded into a map of navmesh info pointers 
               // to indices. That is: the navmesh in a road marker is used to look up an already-loaded 
               // NVMI, and then the pointer to that NVMI (or nullptr if there is none) is associated with 
               // the index. Road marker list items, then, can override on an individual basis, and these 
               // overrides are per-navmesh; and if road marker list items target separate navmeshes that 
               // each lack any previously-seen NVMI, then those list items are applied to null and will 
               // override each other. (We do not emulate the "no NVMI to null" behavior, as this would 
               // require use info generation to actually check road markers against all seen NVMIs. I'd 
               // rather not handle that case until I have a better understanding of the system as a whole; 
               // in partciular, a road marker which refers to a navmesh that lacks NVMI seems like an error 
               // we should endeavor to correct, though we don't emit a warning for it at this time.)
               //
               this->precomputed_paths.load(subrecord, intfc, *this);
               {  // road markers
                  uint32_t count = 0;
                  subrecord.read(count);
                  for (uint32_t i = 0; i < count; ++i) {
                     auto& entry = this->road_markers.emplace_back();
                     entry.is_active_file_data = is_active_file;

                     if (auto& form = entry.navmesh; subrecord.read(form)) {
                        intfc.warn_if_ref_is_wrong_type(form, form_type::navmesh, subrecord.signature());
                     }
                     subrecord.read(entry.index);
                  }
               }
               break;
            case 'NVSI':
               //
               // In-game and in the CK, these lists are coalesced across all NAVI records in all files,
               // used to build a hash-set of deleted navmeshes. The CK warns if it sees in any NAVI/NVSI 
               // a navmesh already in the hash-set.
               // 
               // We bifurcate the deleted navmesh list: we have a list of all deleted navmeshes seen in 
               // masters, and a list of all deleted navmeshes seen in the active file. This is so that 
               // we can update the latter list (and this form's use info) properly on save.
               //
               {
                  std::vector<form_reference_t> list;
                  {
                     size_t count = subrecord.size() / 4;
                     list.resize(count);
                     for (size_t i = 0; i < count; ++i) {
                        auto& form = list[i];
                        subrecord.unchecked_read(form);
                        intfc.warn_if_ref_is_wrong_type(form, form_type::navmesh, subrecord.signature());
                     }
                  }

                  auto& list  = is_active_file  ? this->deleted_navmeshes.active_file : this->deleted_navmeshes.masters;
                  auto& other = !is_active_file ? this->deleted_navmeshes.active_file : this->deleted_navmeshes.masters;

                  size_t count = subrecord.size() / 4;
                  list.reserve(list.size() + count);
                  for (size_t i = 0; i < count; ++i) {
                     form_reference_t form;
                     subrecord.unchecked_read(form);
                     intfc.warn_if_ref_is_wrong_type(form, form_type::navmesh, subrecord.signature());
                     if (!form)
                        continue;

                     bool already_present = false;
                     {
                        auto it = std::find(list.begin(), list.end(), form);
                        if (it != list.end()) {
                           already_present = true;
                        } else {
                           auto it = std::find(other.begin(), other.end(), form);
                           if (it != other.end())
                              already_present = true;
                        }
                     }
                     if (already_present) {
                        specific_load_warnings::navmesh_may_be_multiply_deleted notice(
                           intfc.target_stub,
                           *form.get_form_stub()
                        );
                        intfc.log_load_warning(notice);
                        //
                        // Note that we can't discard duplicate entries here, in part because our use info 
                        // generation code doesn't pay any attention to whether duplicate entries exist, and 
                        // we need to maintain parity between use-info-generation and load.
                        //
                     }
                     list.push_back(form);
                  }
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void NavMeshInfoMap::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'NVER':
               break;
            case 'NVMI':
               {
                  auto& dst_opt = uib.get_form_specific_data()->by_form_type.navmesh_info_map;
                  auto& dst     = dst_opt.has_value() ? dst_opt.value() : dst_opt.emplace();

                  auto& uses = dst.navmesh_info;
                  navmesh_info_collection::generate_use_info(subrecord, uses);
               }
               break;
            case 'NVPP':
               {
                  auto& dst_opt = uib.get_form_specific_data()->by_form_type.navmesh_info_map;
                  auto& dst     = dst_opt.has_value() ? dst_opt.value() : dst_opt.emplace();

                  auto& uses = dst.precomputed_paths;
                  precomputed_path_collection::generate_use_info(subrecord, uses);
               }
               {
                  uint32_t count = 0;
                  subrecord.read(count);
                  for (uint32_t i = 0; i < count; ++i) {
                     form_id_t target;
                     subrecord.read(target);
                     uib.add_outbound_reference(target);
                     subrecord.skip_bytes(4);
                  }
               }
               break;
            case 'NVSI':
               {
                  auto& dst_opt = uib.get_form_specific_data()->by_form_type.navmesh_info_map;
                  auto& dst     = dst_opt.has_value() ? dst_opt.value() : dst_opt.emplace();

                  auto&  uses  = dst.deleted_navmeshes;

                  size_t count = subrecord.size() / 4;
                  for (size_t i = 0; i < count; ++i) {
                     form_id_t form;
                     subrecord.unchecked_read(form);
                     uses.insert(form);
                  }
               }
               break;
         }
      }
   }
   void NavMeshInfoMap::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (NavMeshInfoMap*)out;
      
      copy->navmesh_infos.clone_from(this->navmesh_infos, *copy);
      copy->precomputed_paths.clone_from(this->precomputed_paths, *copy);

      {
         auto&  src_list = this->road_markers;
         auto&  dst_list = copy->road_markers;
         size_t size     = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            auto& src_item = src_list[i];
            auto& dst_item = dst_list[i];
            dst_item.is_active_file_data = src_item.is_active_file_data;
            dst_item.navmesh.set(*copy, src_item.navmesh);
            dst_item.index = src_item.index;
         }
      }

      copy_form_reference_list(*copy, copy->deleted_navmeshes.masters,     this->deleted_navmeshes.masters);
      copy_form_reference_list(*copy, copy->deleted_navmeshes.active_file, this->deleted_navmeshes.active_file);
   }
   void NavMeshInfoMap::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      {
         auto& subrecord = record.open_next_subrecord('NVER');
         subrecord.write((uint32_t)12);
         subrecord.close();
      }

      this->navmesh_infos.save_all(record, intfc);

      // NVPP
      {
         size_t pp_to_save = this->precomputed_paths.paths.size();
         size_t rm_to_save = 0;
         for (auto& item : this->road_markers)
            if (item.is_active_file_data)
               ++rm_to_save;
         if (pp_to_save || rm_to_save) {
            auto& subrecord = record.open_next_subrecord('NVPP');
            this->precomputed_paths.save(subrecord, intfc);
            subrecord.write((uint32_t)rm_to_save);
            for (auto& item : this->road_markers) {
               if (!item.is_active_file_data)
                  continue;
               auto& subrecord = record.get_current_subrecord();
               subrecord.write(item.navmesh);
               subrecord.write(item.index);
            }
            subrecord.close();
         }
      }

      // NVSI
      if (_is_defined_and_overridden_in_different_masters(this->stub)) {
         //
         // Update the active-file list of deleted navmeshes, using similar logic to the CK: 
         // find all navmeshes that are deleted, and aren't defined and overridden by two 
         // different master-flagged files.
         //
         auto items_to_write = _find_all_deleted_navmeshes_of_interest(this->stub.get_owning_load_order());
         {
            const auto&  src_list = items_to_write;
            auto&        dst_list = this->deleted_navmeshes.active_file;
            const size_t src_size = src_list.size();
            const size_t dst_size = dst_list.size();
            if (dst_size < src_size)
               dst_list.resize(src_size);
            for (size_t i = 0; i < src_size; ++i) {
               dst_list[i].set(*this, src_list[i]);
            }
            if (dst_size > src_size) {
               for (size_t i = src_size; i < dst_size; ++i) {
                  dst_list[i].set(*this, nullptr);
               }
               dst_list.resize(src_size);
            }
         }
         if (!items_to_write.empty()) {
            auto& subrecord = record.open_next_subrecord('NVSI');
            for (auto* stub : items_to_write)
               subrecord.write(stub);
            subrecord.close();
         }
      } else {
         clear_form_reference_list(this->deleted_navmeshes.active_file, *this);
      }
   }
   void NavMeshInfoMap::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->navmesh_infos.sever_outbound_references_to(other, *this);
      this->precomputed_paths.sever_outbound_references_to(other, *this);

      for (auto& item : this->road_markers) {
         item.navmesh.clear_if(*this, other);
      }

      remove_form_from_reference_list(this->deleted_navmeshes.masters, other, *this);
      remove_form_from_reference_list(this->deleted_navmeshes.active_file, other, *this);
   }
   void NavMeshInfoMap::_clear_impl() noexcept {
      this->navmesh_infos.clear(*this);
      this->precomputed_paths.clear(*this);

      for (auto& item : this->road_markers) {
         item.navmesh.set(*this, nullptr);
      }
      this->road_markers.clear();

      clear_form_reference_list(this->deleted_navmeshes.masters, *this);
      clear_form_reference_list(this->deleted_navmeshes.active_file, *this);
   }
}