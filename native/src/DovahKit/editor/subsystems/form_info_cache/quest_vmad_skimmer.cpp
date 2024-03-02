#include "./quest_vmad_skimmer.h"
#include "dovah/data/papyrus/helpers/name_equals.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/components/papyrus.h"
#include "editor/subsystems/papyrus/core.h"

namespace {
   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
   }
}

namespace dovahkit::subsystems::form_info_cache {
   bool quest_vmad_skimmer::skim_subrecord(const dovah::form_stub& quest, dovah::tes_file_reading::subrecord& subrecord) {
      switch (subrecord.signature()) {
         case 'VMAD':
            this->skim_vmad(quest, subrecord);
            return true;
         case 'ALST':
         case 'ALLS':
            this->skim_alias_header(quest, subrecord);
            this->in_alias = true;
            return true;
      }
      if (this->in_alias) {
         if (subrecord.signature() == 'ALED') {
            this->in_alias = false;
         }
         return true;
      }
      return false;
   }

   void quest_vmad_skimmer::skim_alias_header(const dovah::form_stub& quest, dovah::tes_file_reading::subrecord& subrecord) {
      uint32_t id;
      if (!subrecord.read(id))
         return;
      if (id > 0xFFFE)
         return;
      this->valid_alias_ids.push_back(id);
   }
   void quest_vmad_skimmer::skim_vmad(const dovah::form_stub& quest, dovah::tes_file_reading::subrecord& subrecord) {
      vmad::attachment_header vmad_header;
      auto result = vmad::attachment_data::skim_vmad_for_scriptnames(subrecord, vmad_header, this->attached, this->deleted);
      if (!result)
         return;

      if (subrecord.is_at_end())
         return;
      
      {
         uint8_t unknown;
         if (!subrecord.read(unknown))
            return;
         /*// Our quest loader doesn't enforce this; apparently the game does?
         if (unknown != 2)
            return;
         //*/
      }
      {  // Log entry fragments
         uint16_t count;
         if (!subrecord.read(count))
            return;
         subrecord.skip_length_prefixed_string<2>();
         for (uint16_t i = 0; i < count; ++i) {
            subrecord.skip_bytes(9);
            subrecord.skip_length_prefixed_string<2>();
            subrecord.skip_length_prefixed_string<2>();
         }
         if (subrecord.is_at_end())
            return;
      }
      //
      // Read alias scripts.
      //
      uint16_t count;
      if (!subrecord.read(count))
         return;
      for (uint16_t i = 0; i < count; ++i) {
         vmad::property_object_value owner;
         owner.load(vmad_header, subrecord);
         if (owner.form != &quest) {
            vmad::attachment_data::skip_use_info(subrecord);
            continue;
         }
         //
         // Load alias script data:
         //
         std::vector<std::string> attached;
         std::vector<std::string> deleted;
         auto result = vmad::attachment_data::skim_vmad_for_scriptnames(subrecord, vmad_header, attached, deleted);
         if (!result)
            break;

         std::vector<std::string> retained;
         for (auto& a : attached) {
            bool lost = false;
            for (const auto& b : deleted) {
               if (dovah::papyrus::helpers::name_equals(a, b)) {
                  lost = true;
                  break;
               }
            }
            if (!lost)
               retained.push_back(std::move(a));
         }
         if (retained.empty())
            continue;

         per_alias* subject = nullptr;
         for (auto& item : this->aliases) {
            if (item.alias_id == owner.alias_id) {
               subject = &item;
               break;
            }
         }
         if (!subject) {
            subject = &this->aliases.emplace_back(per_alias{ .alias_id = owner.alias_id });
         }
         //
         for (const auto& item : retained) {
            bool exists = false;
            for (const auto& prior : subject->scriptnames) {
               if (dovah::papyrus::helpers::name_equals(item, prior)) {
                  exists = true;
                  break;
               }
            }
            if (!exists)
               subject->scriptnames.push_back(std::move(item));
         }
      }
   }

   cached_vmad_info quest_vmad_skimmer::bake() {
      auto& papyrus = dovahkit::subsystems::papyrus::core::get();

      cached_vmad_info out;
      
      for (auto& name : this->attached) {
         out.attached.push_back(
            papyrus.know_script_via_vmad_scan(name)
         );
      }
      for (auto& name : this->deleted) {
         out.deleted.push_back(
            papyrus.know_script_via_vmad_scan(name)
         );
      }

      for (auto& alias_info : this->aliases) {
         bool valid = false;
         for (auto id : this->valid_alias_ids) {
            if (alias_info.alias_id == id) {
               valid = true;
               break;
            }
         }
         if (!valid)
            continue;
         
         for (auto& name : alias_info.scriptnames) {
            out.aliases.push_back(
               papyrus.know_script_via_vmad_scan(name)
            );
         }
      }

      return out;
   }
}