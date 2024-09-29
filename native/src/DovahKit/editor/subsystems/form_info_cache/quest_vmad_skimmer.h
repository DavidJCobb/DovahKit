#pragma once
#include <cstdint>
#include <string>
#include "./cached_data/attached_scripts.h"

namespace dovah {
   namespace tes_file_reading {
      class subrecord;
   }
   namespace loaded_forms::components::papyrus {
      class attachment_data;
   }
   class form_stub;
}

namespace dovahkit::subsystems::form_info_cache {
   struct quest_vmad_skimmer {
      constexpr quest_vmad_skimmer() {}

      struct per_alias {
         uint16_t alias_id = 0xFFFF;
         std::vector<std::string> scriptnames;
      };

      std::vector<std::string> attached;
      std::vector<std::string> deleted;
      std::vector<per_alias>   aliases;

      std::vector<uint16_t> valid_alias_ids;

      bool in_alias = false;

      constexpr bool empty() const noexcept {
         return this->attached.empty() && this->deleted.empty() && this->aliases.empty();
      }

      // Returns `true` if the subrecord's signature is relevant to, and consumed by, this skimmer.
      bool skim_subrecord(const dovah::form_stub& quest, dovah::tes_file_reading::subrecord& subrecord);

      void skim_alias_header(const dovah::form_stub& quest, dovah::tes_file_reading::subrecord& alias_subrecord); // ALLS or ALST
      void skim_vmad(const dovah::form_stub& quest, dovah::tes_file_reading::subrecord& vmad);

      cached_data::attached_scripts bake();
   };
}