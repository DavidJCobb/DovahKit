#pragma once
#include "../../papyrus/known_script_ptr.h"

namespace dovah {
   namespace tes_file_reading {
      class subrecord;
   }
   namespace loaded_forms::components::papyrus {
      class attachment_data;
   }
}

namespace dovahkit::subsystems::form_info_cache::cached_data {
   struct attached_scripts {
      constexpr attached_scripts() {}
      attached_scripts(dovah::tes_file_reading::subrecord& vmad);
      attached_scripts(const dovah::loaded_forms::components::papyrus::attachment_data&);

      std::vector<papyrus::known_script_ptr> attached;
      std::vector<papyrus::known_script_ptr> deleted; // for REFRs choosing not to inherit a base-form script
      std::vector<papyrus::known_script_ptr> aliases; // scripts attached to quest aliases

      constexpr bool empty() const noexcept {
         return this->attached.empty() && this->deleted.empty() && this->aliases.empty();
      }

      constexpr bool operator==(const attached_scripts&) const;
   };
}

#include "./attached_scripts.inl"