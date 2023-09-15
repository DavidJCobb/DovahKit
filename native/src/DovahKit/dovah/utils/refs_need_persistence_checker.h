#pragma once
#include "helpers/passkey.h"

namespace dovah {
   class file_load_order;
   class form_stub;
   namespace tes_file_writing {
      class file_writer;
   }
}

namespace dovah {
   class refs_need_persistence_checker {
      protected:
         using file_writing_passkey = cobb::passkey<refs_need_persistence_checker, tes_file_writing::file_writer>;

      protected:
         file_load_order& load_order;
         bool grabbed_dobj = false;
         bool during_save  = false; // if `true`, this is being used by the save process to check whether REFR persistence flags need updating

         struct {
            const form_stub* dragon_marker_crash = nullptr; // DCZM
            const form_stub* dragon_marker_land  = nullptr; // DLZM
            const form_stub* persist_all         = nullptr; // PLOC
         } dobj;

         void _grab_default_objects();

         static bool _actor_is_unique(form_stub& actor_base);
         static bool _quest_targets_unique_actor(form_stub& quest_stub, const form_stub& unique_actor_base_stub);

      public:
         refs_need_persistence_checker(file_load_order&);

         // Throws if called during load/save by anything that isn't part of DovahKit serialization internals.
         bool check_ref(form_stub& refr);

         void _set_is_during_save(file_writing_passkey);
   };
}