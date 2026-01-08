#include "./objects.h"

namespace dovah::loaded_forms::structs::region::generable_content {
   void raw_object_collection::clear(dovah::loaded_forms::Form& my_containing_form) {
      for (auto& item : this->objects) {
         item.form.set(my_containing_form, nullptr);
      }
      this->objects.clear();
   }
   void raw_object_collection::clone_from(dovah::loaded_forms::Form& my_containing_form, const raw_object_collection& src) {
      this->clear(my_containing_form);
      const size_t size = src.objects.size();
      this->objects.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& src_obj = src.objects[i];
         auto& dst_obj = this->objects[i];
         dst_obj.form.set(my_containing_form, src_obj.form);
         dst_obj.parent_index = src_obj.parent_index;
         dst_obj.params       = src_obj.params;
      }
   }
   void raw_object_collection::sever_references_to(dovah::loaded_forms::Form& my_containing_form, dovah::form_stub& stub) {
      auto&  list = this->objects;
      size_t size = list.size();
      for(size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.form.get_form_stub() == &stub) {
            item.form.set(my_containing_form, nullptr);
            list.erase(list.begin() + i);
            --i;
            --size;
            continue;
         }
      }
   }
}