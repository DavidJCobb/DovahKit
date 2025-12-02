#pragma once
namespace dovah {
   namespace load_order_interfaces {
      class form_load;
   }
   namespace tes_file_reading {
      class file_loader;
      class subrecord;
   }
}

namespace dovah::utils {
   //
   // Some form types need to remember when they first saw a given subrecord. This is 
   // mainly needed for IDLE/ANAM and all its associated jank.
   //
   struct subrecord_data_position {
      static subrecord_data_position from_loader(tes_file_reading::subrecord&, load_order_interfaces::form_load&);

      const tes_file_reading::file_loader* source_file = nullptr;
      struct _ {
         constexpr bool operator==(const _&) const noexcept = default;

         size_t of_record    = 0; // record offset within containing file
         size_t of_subrecord = 0; // subrecord offset within record body (in case record was compressed and had multiple of this subrecord)
      } offsets;

      constexpr bool operator==(const subrecord_data_position&) const noexcept = default;
   };
}