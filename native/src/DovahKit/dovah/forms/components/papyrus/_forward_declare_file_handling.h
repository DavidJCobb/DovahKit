#pragma once

namespace dovah {
   namespace tes_file_reading {
      class record;
      class subrecord;
   }
   namespace tes_file_writing {
      class record;
      class subrecord;
   }
   namespace load_order_interfaces {
      class form_load;
      class form_save;
   }
   class form_stub_use_info_builder;

   using tes_record_reader    = tes_file_reading::record;
   using tes_subrecord_reader = tes_file_reading::subrecord;

   using tes_record_writer    = tes_file_writing::record;
   using tes_subrecord_writer = tes_file_writing::subrecord;
}