#pragma once
#include <QString>
namespace dovah {
   namespace loaded_forms {
      namespace structs::region::generable_content {
         class map;
      }
      class Region;
   }
   class form_stub;
}

namespace ui::types::regions::generable_content {
   class map {
      public:
         using backend_collection_type = dovah::loaded_forms::structs::region::generable_content::map;

      public:
         QString name;

      public:
         void clear();
         void import_data(const dovah::loaded_forms::Region&);
         void import_data(const backend_collection_type&);
         void export_data(dovah::loaded_forms::Region&, backend_collection_type&) const;
   };
}