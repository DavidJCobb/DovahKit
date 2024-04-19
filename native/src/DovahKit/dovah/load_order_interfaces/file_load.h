#pragma once

namespace dovah {
   namespace tes_file_reading {
      class file_loader;
   }
   struct detailed_notice;
   class file_load_order;
}

namespace dovah::load_order_interfaces {
   class file_load {
      friend class file_load_order;
      friend class tes_file_reading::file_loader;
      public:
         file_load_order& owner;

         void log_load_warning(detailed_notice&);
         void log_load_error(detailed_notice&);

      protected:
         file_load(file_load_order& o) : owner(o) {}
   };
}