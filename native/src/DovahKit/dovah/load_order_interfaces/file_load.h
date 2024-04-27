#pragma once

namespace dovah {
   namespace notices {
      class base_file_load_warning;
   }
   namespace tes_file_reading {
      class file_loader;
   }
   class file_load_order;
}

namespace dovah::load_order_interfaces {
   class file_load {
      friend class file_load_order;
      friend class tes_file_reading::file_loader;
      public:
         file_load_order& owner;

         void log_warning(const notices::base_file_load_warning&);

      protected:
         file_load(file_load_order& o) : owner(o) {}
   };
}