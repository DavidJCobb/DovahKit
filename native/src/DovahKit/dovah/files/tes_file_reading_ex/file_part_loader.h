#pragma once
#include "basic_reader.h"
#include "../file_load_order.h"

namespace dovah::tes_file_reading {
   class file_loader;

   class file_part_loader : basic_reader {
      using interface_t = load_order_interfaces::file_load;
      public:
         file_part_loader(file_loader&);
         //
         bool load_record_at(uint32_t pos);
         //
         object_type next_record_or_group(); // only called during the initial file read
         bool        next_subrecord(); // called after the initial file read, when loading a form_stub's full content
         //
      protected:
         file_loader& owner;
         interface_t  load_interface;
   };
}