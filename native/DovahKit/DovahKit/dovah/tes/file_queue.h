#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace dovah::tes {
   //
   // A flat list of data files that we would like to load, without any attempt made 
   // to analyze their dependencies, sort them based on their flags, or any such.
   //
   class file_queue {
      public:
         using file_name_type = std::string;
         using file_list_type = std::vector<file_name_type>;

         static constexpr const size_t index_of_none = (size_t)-1;

      protected:
         file_list_type        files;
         file_name_type        active_file_name;
         std::filesystem::path data_directory;

      public:
         void insert(const file_name_type&);
         void insert(file_name_type&&);
         void remove(const file_name_type&);
         void set_active_file_name(const file_name_type&);

         constexpr bool contains(const file_name_type&) const;
         constexpr size_t index_of(const file_name_type&) const;

         constexpr const file_name_type& get_active_file_name() const noexcept { return this->active_file_name; }
         constexpr const file_list_type& get_file_list() const noexcept { return this->files; }
   };
}

#include "./file_queue.inl"