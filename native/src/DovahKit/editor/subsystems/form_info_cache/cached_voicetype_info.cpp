#include "./cached_voicetype_info.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/Voicetype.h"
#include "dovah/core.h"

namespace {
   using loaded_form_type = dovah::loaded_forms::Voicetype;
}

namespace dovahkit::subsystems::form_info_cache {
   void cached_voicetype_info::skim_subrecord(dovah::tes_file_reading::subrecord& subrecord) {
      switch (subrecord.signature()) {
         case 'DNAM':
            {
               loaded_form_type::voicetype_flags_t flags = 0;
               if (subrecord.read(flags)) {
                  this->allow_default_dialogue = flags & loaded_form_type::voicetype_flag::allow_default_dialogue;
                  this->female                 = flags & loaded_form_type::voicetype_flag::female;
               }
            }
            break;
      }
   }
   bool cached_voicetype_info::update(const dovah::loaded_forms::Voicetype& src) {
      auto flags = src.voicetype_flags;

      bool allow  = flags & loaded_form_type::voicetype_flag::allow_default_dialogue;
      bool female = flags & loaded_form_type::voicetype_flag::female;

      bool changed = false;
      if (this->female != female)
         changed = true;
      if (this->allow_default_dialogue != allow)
         changed = true;
      
      this->allow_default_dialogue = allow;
      this->female = female;

      return changed;
   }
}