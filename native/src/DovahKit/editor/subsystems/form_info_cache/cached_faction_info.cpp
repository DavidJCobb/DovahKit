#include "./cached_faction_info.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/Faction.h"
#include "dovah/core.h"

namespace {
   using loaded_form_type = dovah::loaded_forms::Faction;
}

namespace dovahkit::subsystems::form_info_cache {
   void cached_faction_info::skim_subrecord(dovah::tes_file_reading::subrecord& subrecord) {
      switch (subrecord.signature()) {
         case 'DATA':
            {
               decltype(loaded_form_type::faction_flags) flags = 0;
               if (subrecord.read(flags)) {
                  this->tracks_crime = flags & loaded_form_type::faction_flag::track_crime;
               }
            }
            break;
      }
   }
   bool cached_faction_info::update(const dovah::loaded_forms::Faction& src) {
      auto flags = src.faction_flags;

      bool tracks_crime = flags & loaded_form_type::faction_flag::track_crime;

      bool changed = false;
      if (this->tracks_crime != tracks_crime)
         changed = true;
      
      this->tracks_crime = tracks_crime;

      return changed;
   }
}