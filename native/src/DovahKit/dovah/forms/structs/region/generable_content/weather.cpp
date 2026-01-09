#include "./weather.h"

namespace dovah::loaded_forms::structs::region::generable_content {
   void weather_collection::clear(dovah::loaded_forms::Form& my_containing_form) {
      for (auto& item : this->weathers) {
         item.weather.set(my_containing_form, nullptr);
         item.global.set(my_containing_form, nullptr);
      }
      this->weathers.clear();
   }
   void weather_collection::clone_from(dovah::loaded_forms::Form& my_containing_form, const weather_collection& src) {
      this->clear(my_containing_form);
      const size_t size = src.weathers.size();
      this->weathers.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& src_obj = src.weathers[i];
         auto& dst_obj = this->weathers[i];
         dst_obj.weather.set(my_containing_form, src_obj.weather);
         dst_obj.global.set(my_containing_form, src_obj.global);
         dst_obj.chance = src_obj.chance;
      }
   }
   void weather_collection::sever_references_to(dovah::loaded_forms::Form& my_containing_form, dovah::form_stub& stub) {
      auto&  list = this->weathers;
      size_t size = list.size();
      for(size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.weather.get_form_stub() == &stub) {
            item.weather.set(my_containing_form, nullptr);
            item.global.set(my_containing_form, nullptr);
            list.erase(list.begin() + i);
            --i;
            --size;
            continue;
         }
         item.global.clear_if(my_containing_form, stub);
      }
   }

   void weather_collection::copy_insert_from(dovah::loaded_forms::Form& my_containing_form, const weather_collection& src) {
      size_t src_size = src.weathers.size();
      size_t dst_size = this->weathers.size();
      this->weathers.reserve(src_size + dst_size);
      for (auto& src_item : src.weathers) {
         bool found = false;
         for (auto& dst_item : this->weathers) {
            if (dst_item.weather == src_item.weather) {
               found = true;
               dst_item.chance = src_item.chance;
               dst_item.global.set(my_containing_form, src_item.global);
               break;
            }
         }
         if (found)
            continue;
         auto& dst_item = this->weathers.emplace_back();
         dst_item.chance = src_item.chance;
         dst_item.global.set(my_containing_form, src_item.global);
         dst_item.weather.set(my_containing_form, src_item.weather);
      }
   }
}