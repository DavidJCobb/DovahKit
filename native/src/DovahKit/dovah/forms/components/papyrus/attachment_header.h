#pragma once
#include <cstdint>
#include "./_forward_declare_file_handling.h"

namespace dovah::loaded_forms::components::papyrus {
   struct attachment_header {
      int16_t version       = 5;
      int16_t object_format = 2; // format of "object" property values
      
      bool load(tes_subrecord_reader&);
      static void skip(tes_subrecord_reader&);
      void save(tes_subrecord_writer&) const;
   };
}