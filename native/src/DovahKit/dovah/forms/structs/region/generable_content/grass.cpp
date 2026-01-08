#include "./grass.h"
#include "../../../../form_stub.h"

namespace dovah::loaded_forms::structs::region::generable_content {
   void grass_collection::clear(dovah::loaded_forms::Form& my_containing_form) {
      for (auto& mapping : this->entries) {
         mapping.grass.set(my_containing_form, nullptr);
         mapping.land_texture.set(my_containing_form, nullptr);
      }
      this->entries.clear();
   }
   void grass_collection::clone_from(dovah::loaded_forms::Form& my_containing_form, const grass_collection& src) {
      this->clear(my_containing_form);
      const size_t size = src.entries.size();
      this->entries.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& src_obj = src.entries[i];
         auto& dst_obj = this->entries[i];
         dst_obj.grass.set(my_containing_form, src_obj.grass);
         dst_obj.land_texture.set(my_containing_form, src_obj.land_texture);
      }
   }
   void grass_collection::sever_references_to(dovah::loaded_forms::Form& my_containing_form, dovah::form_stub& stub) {
      auto&  list = this->entries;
      size_t size = list.size();
      for(size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.grass.get_form_stub() == &stub || item.land_texture.get_form_stub() == &stub) {
            item.grass.set(my_containing_form, nullptr);
            item.land_texture.set(my_containing_form, nullptr);
            list.erase(list.begin() + i);
            --i;
            --size;
         }
      }
   }
}