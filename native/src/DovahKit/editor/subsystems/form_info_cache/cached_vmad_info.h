#pragma once
#include "../papyrus/known_script_ptr.h"

namespace dovah {
   namespace tes_file_reading {
      class subrecord;
   }
   namespace loaded_forms::components::papyrus {
      class attachment_data;
   }
}

namespace dovahkit::subsystems::form_info_cache {
   struct cached_vmad_info {
      constexpr cached_vmad_info() {}
      cached_vmad_info(dovah::tes_file_reading::subrecord& vmad);
      cached_vmad_info(const dovah::loaded_forms::components::papyrus::attachment_data&);

      std::vector<papyrus::known_script_ptr> attached;
      std::vector<papyrus::known_script_ptr> deleted; // for REFRs choosing not to inherit a base-form script

      constexpr bool empty() const noexcept {
         return this->attached.empty() && this->deleted.empty();
      }

      constexpr bool operator==(const cached_vmad_info&) const;
   };
}

#include "./cached_vmad_info.inl"