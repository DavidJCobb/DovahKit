#pragma once

namespace dovah {
   namespace loaded_forms {
      class ActorBase;
   }
   namespace tes_file_reading {
      class subrecord;
   }
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   struct actor_base {
      constexpr actor_base() {}

      bool female     = false;
      bool summonable = false;
      bool unique     = false;

      void skim_subrecord(dovah::tes_file_reading::subrecord&);

      // Returns true if anything has changed.
      bool update(const dovah::loaded_forms::ActorBase&);
   };
}