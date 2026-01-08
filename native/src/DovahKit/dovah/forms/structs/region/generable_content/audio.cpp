#include "./audio.h"

namespace dovah::loaded_forms::structs::region::generable_content {
   void audio::clear(dovah::loaded_forms::Form& my_containing_form) {
      this->music.set(my_containing_form, nullptr);
      for (auto& item : this->ambient_sounds)
         item.form.set(my_containing_form, nullptr);
      this->ambient_sounds.clear();
   }
   void audio::clone_from(dovah::loaded_forms::Form& my_containing_form, const audio& src) {
      this->clear(my_containing_form);
      this->music.set(my_containing_form, src.music);

      const size_t size = src.ambient_sounds.size();
      this->ambient_sounds.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& src_obj = src.ambient_sounds[i];
         auto& dst_obj = this->ambient_sounds[i];
         dst_obj.form.set(my_containing_form, src_obj.form);
         dst_obj.chance = src_obj.chance;
         dst_obj.flags  = src_obj.flags;
      }
   }
   void audio::sever_references_to(dovah::loaded_forms::Form& my_containing_form, dovah::form_stub& stub) {
      auto&  list = this->ambient_sounds;
      size_t size = list.size();
      for(size_t i = 0; i < size; ++i) {
         auto& mapping = list[i];
         if (mapping.form.get_form_stub() == &stub) {
            mapping.form.set(my_containing_form, nullptr);
            list.erase(list.begin() + i);
            --i;
            --size;
            continue;
         }
      }
      this->music.clear_if(my_containing_form, stub);
   }
}