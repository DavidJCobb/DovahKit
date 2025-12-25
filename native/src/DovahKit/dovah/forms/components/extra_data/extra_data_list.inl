#pragma once
#include "./extra_data_list.h"
#include "./extra_data.h"
#include "./types/class_array.h"

namespace dovah::loaded_forms::components {
   template<typename T> requires all_extra_data_types::contains_type<T>
   const T* extra_data_list::get() const noexcept {
      for (auto* extra : this->content)
         if (extra->typecode == all_extra_data_types::index_of_type<T>)
            return (T*)extra;
      return nullptr;
   }

   template<typename T> requires all_extra_data_types::contains_type<T>
   T* extra_data_list::get() noexcept {
      return const_cast<T*>(std::as_const(*this).get<T>());
   }

   template<typename T> requires all_extra_data_types::contains_type<T>
   T* extra_data_list::get_or_create() noexcept {
      auto* extra = get<T>();
      if (extra)
         return extra;
      auto& ptr = this->content.emplace_back();
      ptr = new T;
      return static_cast<T*>(ptr);
   }

   template<typename T> requires all_extra_data_types::contains_type<T>
   void extra_data_list::remove(loaded_forms::Form& my_owner) noexcept {
      for (size_t i = 0; i < this->content.size(); ++i) {
         auto* extra = this->content[i];
         if (extra->typecode != all_extra_data_types::index_of_type<T>)
            continue;
         extra->clear_contained_formIDs(my_owner);
         this->content.erase(this->content.begin() + i);
         return;
      }
   }
}