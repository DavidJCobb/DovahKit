#pragma once
#include <vector>
namespace dovah {
   namespace loaded_forms {
      namespace structs::region::generable_content {
         class audio;
      }
      class Region;
   }
   class form_stub;
}

namespace ui::types::regions::generable_content {
   class audio {
      public:
         using backend_collection_type = dovah::loaded_forms::structs::region::generable_content::audio;

         struct ambient_sound {
            dovah::form_stub* sound = nullptr; // -> SOUN/SNDR
            float chance = 0.0F;
            struct {
               bool pleasant = false;
               bool cloudy = false;
               bool rainy = false;
               bool snowy = false;
            } weather;
         };

      public:
         std::vector<ambient_sound> ambient_sounds;
         dovah::form_stub* music_type = nullptr;

         constexpr bool empty() const noexcept;

      public:
         void clear();
         void import_data(const dovah::loaded_forms::Region&);
         void import_data(const backend_collection_type&);
         void export_data(dovah::loaded_forms::Region&, backend_collection_type&) const;
   };
}

#include "./audio.inl"