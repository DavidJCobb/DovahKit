#include "./precomputed_path.h"
#include "../../_common_cpp.h"

#include "../../../notices/form_load_warnings/by_form_type/navmesh_info_map/precomputed_path_has_gaps.h"
#include "../../../notices/form_load_warnings/by_form_type/navmesh_info_map/precomputed_path_is_empty.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::navmesh_info_map;
   }
}

namespace dovah::loaded_forms::structs::navmesh_info_map {
   void precomputed_path::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc, size_t which_am_i) {
      uint32_t size = 0;
      subrecord.read(size);
      this->navmeshes.reserve(size);
      if (size == 0) {
         specific_load_warnings::precomputed_path_is_empty notice(
            intfc.target_stub,
            which_am_i
         );
         intfc.log_load_warning(notice);
      }
      bool any_missing = false;
      for (uint32_t i = 0; i < size; ++i) {
         auto& form = this->navmeshes.emplace_back();
         if (subrecord.read(form)) {
            intfc.warn_if_ref_is_wrong_type(form, form_type::navmesh, subrecord.signature());
            if (!form) {
               any_missing = true;
            }
         }
      }
      if (any_missing) {
         //
         // Warn about a path with gaps in it.
         //
         form_stub* endpoint_a = nullptr;
         form_stub* endpoint_b = nullptr;
         bool endpoint_a_is_end = false;
         bool endpoint_b_is_end = false;
         if (endpoint_a = this->navmeshes.front().get_form_stub()) {
            endpoint_a_is_end = true;
         }
         if (endpoint_b = this->navmeshes.back().get_form_stub()) {
            endpoint_b_is_end = true;
         }
         if (!endpoint_a_is_end || !endpoint_b_is_end) {
            auto& list = this->navmeshes;
            if (!endpoint_a_is_end) {
               auto it_a = std::find_if(
                  list.begin(),
                  list.end(),
                  [](auto& form) { return !!form; }
               );
               if (it_a != list.end())
                  endpoint_a = it_a->get_form_stub();
            }
            if (!endpoint_b_is_end) {
               auto it_b = std::find_if(
                  list.rbegin(),
                  list.rend(),
                  [](auto& form) { return !!form; }
               );
               if (it_b != list.rend())
                  endpoint_b = it_b->get_form_stub();
            }
         }
         specific_load_warnings::precomputed_path_has_gaps notice(
            intfc.target_stub,
            which_am_i,
            endpoint_a,
            endpoint_a_is_end,
            endpoint_b,
            endpoint_b_is_end
         );
         intfc.log_load_warning(notice);
      }
   }
   /*static*/ void precomputed_path::generate_use_info(tes_subrecord_reader& subrecord, std::vector<form_id_t>& append_to) {
      uint32_t size = 0;
      subrecord.read(size);
      for (uint32_t i = 0; i < size; ++i) {
         form_id_t form;
         if (subrecord.read(form) && form)
            append_to.push_back(form);
      }
   }
   void precomputed_path::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.write((uint32_t)this->navmeshes.size());
      for (auto& form : this->navmeshes)
         subrecord.write(form);
   }

   void precomputed_path::clone_from(const precomputed_path& src, Form& my_owner) noexcept {
      copy_form_reference_list(my_owner, this->navmeshes, src.navmeshes);
   }
   void precomputed_path::clear(Form& my_owner) noexcept {
      clear_form_reference_list(this->navmeshes, my_owner);
   }
   void precomputed_path::sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept {
      remove_form_from_reference_list(this->navmeshes, other, my_owner);
   }
}