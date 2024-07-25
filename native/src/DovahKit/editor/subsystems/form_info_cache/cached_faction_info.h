#pragma once

namespace dovah {
   namespace loaded_forms {
      class Faction;
   }
   namespace tes_file_reading {
      class subrecord;
   }
}

namespace dovahkit::subsystems::form_info_cache {
   struct cached_faction_info {
      constexpr cached_faction_info() {}

      bool tracks_crime = false;

      void skim_subrecord(dovah::tes_file_reading::subrecord&);

      // Returns true if anything has changed.
      bool update(const dovah::loaded_forms::Faction&);
   };
}