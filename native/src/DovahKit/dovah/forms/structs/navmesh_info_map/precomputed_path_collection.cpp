#include "./precomputed_path_collection.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::structs::navmesh_info_map {
   void precomputed_path_collection::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc, Form& my_owner) {
      this->clear(my_owner);

      uint32_t size = 0;
      subrecord.read(size);
      this->paths.reserve(size);
      for (uint32_t i = 0; i < size; ++i) {
         auto& path = this->paths.emplace_back();
         path.load(subrecord, intfc, i);
      }
   }
   /*static*/ void precomputed_path_collection::generate_use_info(tes_subrecord_reader& subrecord, form_specific_use_info_data& append_to) {
      append_to.navmeshes.clear();

      uint32_t size = 0;
      subrecord.read(size);
      for (uint32_t i = 0; i < size; ++i) {
         precomputed_path::generate_use_info(subrecord, append_to.navmeshes);
      }
   }
   void precomputed_path_collection::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.write((uint32_t)this->paths.size());
      for (auto& path : this->paths)
         path.save(subrecord, intfc);
   }

   void precomputed_path_collection::clone_from(const precomputed_path_collection& src, Form& my_owner) noexcept {
      const auto&  src_list = src.paths;
      auto&        dst_list = this->paths;
      const size_t size     = src_list.size();
      const size_t from     = dst_list.size();
      if (size > from) {
         dst_list.resize(size);
      }
      for (size_t i = 0; i < size; ++i) {
         dst_list[i].clone_from(src_list[i], my_owner);
      }
      if (size < from) {
         for (size_t i = size; i < from; ++i)
            dst_list[i].clear(my_owner);
         dst_list.resize(size);
      }
   }
   void precomputed_path_collection::clear(Form& my_owner) noexcept {
      for (auto& path : this->paths)
         path.clear(my_owner);
      this->paths.clear();
   }
   void precomputed_path_collection::sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept {
      bool removed_any = false;
      for (auto& path : this->paths) {
         path.sever_outbound_references_to(other, my_owner);
         if (path.navmeshes.empty())
            removed_any = true;
      }
      if (removed_any) {
         std::erase_if(
            this->paths,
            [](auto& path) {
               return path.navmeshes.empty();
            }
         );
      }
   }
}