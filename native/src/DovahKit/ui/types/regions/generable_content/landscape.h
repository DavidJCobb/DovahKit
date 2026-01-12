#pragma once
#include "ui/types/game_file_path.h"
namespace dovah {
   namespace loaded_forms {
      namespace structs::region::generable_content {
         class landscape;
      }
      class Region;
   }
   class form_stub;
}

namespace ui::types::regions::generable_content {
   class landscape {
      public:
         using backend_collection_type = dovah::loaded_forms::structs::region::generable_content::landscape;

      public:
         game_file_path texture;

      public:
         void clear();
         void import_data(const dovah::loaded_forms::Region&);
         void import_data(const backend_collection_type&);
         void export_data(dovah::loaded_forms::Region&, backend_collection_type&) const;
   };
}