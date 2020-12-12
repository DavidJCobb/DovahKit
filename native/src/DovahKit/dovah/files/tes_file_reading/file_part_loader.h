#pragma once
#include "file_or_file_part_loader.h"

namespace dovah::tes_file_reading {
   class file_loader;

   class file_part_loader : public file_or_file_part_loader {
      friend class file_loader;
      using lo_interface_t = load_order_interfaces::file_load;
      protected:
         virtual file_loader& get_file_loader() const noexcept override final { return this->owner; }
      public:
         file_part_loader(file_loader&, lo_interface_t&);
         //
         bool load_record_at(uint32_t pos);
         //
         object_type next_record_or_group(); // only called during the initial file read
         bool        next_subrecord(); // called after the initial file read, when loading a form_stub's full content
         //
      protected:
         file_loader& owner;
         //
         void _ensure_file();
         void _on_file_close(); // called by file_loader::close
   };
}