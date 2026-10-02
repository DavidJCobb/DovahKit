#include "./file_queue.h"

namespace dovah::tes {
   void file_queue::insert(const file_name_type& name) {
      if (!this->contains(name))
         this->files.push_back(name);
   }
   void file_queue::insert(file_name_type&& name) {
      if (!this->contains(name))
         this->files.push_back(std::move(name));
   }
   void file_queue::remove(const file_name_type& name) {
      auto i = this->index_of(name);
      if (i != index_of_none)
         this->files.erase(this->files.begin() + i);
   }
   void file_queue::set_active_file_name(const file_name_type& name) {
      if (this->active_file_name == name)
         return;
      if (!this->contains(name))
         this->files.push_back(name);
      this->active_file_name = name;
   }
}