#pragma once

namespace dovah {
   namespace tes_file_writing {
      class file_writer;
   }
   namespace utils {
      struct update_location_content; // ONLY for updating LCTN mid-save
   }
   class form_deletion_request; // ONLY when deleting forms mid-save, as part of cleanup
   class refs_need_persistence_checker;
}

namespace dovah::form_stub_passkeys {
   class force_form_load {
      friend dovah::tes_file_writing::file_writer;
      friend dovah::refs_need_persistence_checker;
      friend form_deletion_request;
      friend utils::update_location_content;
      private:
         constexpr force_form_load() {}
   };
}