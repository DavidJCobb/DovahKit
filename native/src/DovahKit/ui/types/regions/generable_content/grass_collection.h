#pragma once
#include <vector>
namespace dovah {
   namespace loaded_forms {
      namespace structs::region::generable_content {
         class grass_collection;
      }
      class Region;
   }
   class form_stub;
}

namespace ui::types::regions::generable_content {
   class grass_collection {
      public:
         using backend_collection_type = dovah::loaded_forms::structs::region::generable_content::grass_collection;

         struct entry {
            dovah::form_stub* grass        = nullptr;
            dovah::form_stub* land_texture = nullptr;
         };

      public:
         std::vector<entry> grasses;

         constexpr bool empty() const noexcept;

      public:
         void clear();
         void import_data(const dovah::loaded_forms::Region&);
         void import_data(const backend_collection_type&);
         void export_data(dovah::loaded_forms::Region&, backend_collection_type&) const;
   };
}

#include "./grass_collection.inl"