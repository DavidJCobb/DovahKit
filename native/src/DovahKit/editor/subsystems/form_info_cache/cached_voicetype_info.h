#pragma once

namespace dovah {
   namespace loaded_forms {
      class Voicetype;
   }
   namespace tes_file_reading {
      class subrecord;
   }
}

namespace dovahkit::subsystems::form_info_cache {
   struct cached_voicetype_info {
      constexpr cached_voicetype_info() {}

      bool allow_default_dialogue = false;
      bool female = false;

      void skim_subrecord(dovah::tes_file_reading::subrecord&);

      // Returns true if anything has changed.
      bool update(const dovah::loaded_forms::Voicetype&);
   };
}