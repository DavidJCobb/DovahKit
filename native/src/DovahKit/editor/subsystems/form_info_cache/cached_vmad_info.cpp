#include "./cached_vmad_info.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/components/papyrus.h"
#include "editor/subsystems/papyrus/core.h"

namespace {
   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
   }
}

namespace dovahkit::subsystems::form_info_cache {
   cached_vmad_info::cached_vmad_info(dovah::tes_file_reading::subrecord& subrecord) {
      auto& papyrus = dovahkit::subsystems::papyrus::core::get();

      std::vector<std::string> attached_scripts;
      std::vector<std::string> deleted_scripts;

      vmad::attachment_data::skim_vmad_for_scriptnames(subrecord, attached_scripts, deleted_scripts);

      cached_vmad_info info;
      for (auto& name : attached_scripts) {
         info.attached.push_back(
            papyrus.know_script_via_vmad_scan(name)
         );
      }
      for (auto& name : deleted_scripts) {
         info.deleted.push_back(
            papyrus.know_script_via_vmad_scan(name)
         );
      }
   }
   cached_vmad_info::cached_vmad_info(const vmad::attachment_data& src) {
      auto& papyrus = dovahkit::subsystems::papyrus::core::get();

      for (auto& script : src.scripts) {
         auto known = papyrus.know_script(script.name);
         switch (script.status) {
            case vmad::script_status::removed:
               this->deleted.push_back(std::move(known));
               break;
            default:
               this->attached.push_back(std::move(known));
               break;
         }
      }
   }
}