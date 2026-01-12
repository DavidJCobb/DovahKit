#pragma once
#include <vector>
namespace dovah {
   namespace loaded_forms {
      namespace structs::region::generable_content {
         class weather_collection;
      }
      class Region;
   }
   class form_stub;
}

namespace ui::types::regions::generable_content {
   class weather_collection {
      public:
         using backend_collection_type = dovah::loaded_forms::structs::region::generable_content::weather_collection;

         static constexpr const size_t index_of_none = (size_t)-1;
         
         struct entry {
            dovah::form_stub* weather         = nullptr;
            float             chance_constant = 0.0F;
            dovah::form_stub* chance_global   = nullptr;
         };

      public:
         std::vector<entry> weathers;

         constexpr bool contains(const dovah::form_stub& weather) const noexcept;
         constexpr size_t index_of(const dovah::form_stub& weather) const noexcept;
         constexpr bool empty() const noexcept;

      public:
         void clear();
         void import_data(const dovah::loaded_forms::Region&);
         void import_data(const backend_collection_type&);
         void export_data(dovah::loaded_forms::Region&, backend_collection_type&) const;
   };
}

#include "./weather_collection.inl"