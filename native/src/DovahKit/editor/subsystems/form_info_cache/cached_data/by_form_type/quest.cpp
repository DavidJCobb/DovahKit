#include "./quest.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/Quest.h"
#include "dovah/core.h"

namespace {
   using loaded_form_type = dovah::loaded_forms::Quest;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   void quest::skim_subrecord(dovah::tes_file_reading::subrecord& subrecord) {
      switch (subrecord.signature()) {
         case 'ENAM':
            subrecord.read_signature(this->event_signature);
            break;
      }
   }
   bool quest::update(const loaded_form_type& src) {
      bool changed = false;
      if (this->event_signature != src.event)
         changed = true;

      return changed;
   }
}