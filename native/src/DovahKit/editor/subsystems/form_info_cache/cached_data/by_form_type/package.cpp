#include "./package.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/Package.h"
#include "dovah/core.h"

namespace {
   using loaded_form_type = dovah::loaded_forms::Package;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   void package::skim_record(dovah::tes_file_reading::record& record) {
      struct {
         loaded_form_type::record_skimmers::legacy_type legacy_type;
      } skimmers;

      while (auto& subrecord = record.next_subrecord()) {
         skimmers.legacy_type.skim_subrecord(subrecord);
      }

      skimmers.legacy_type.finalize();
      if (skimmers.legacy_type.result.has_value()) {
         this->legacy_type = skimmers.legacy_type.result.value();
      }
   }
   bool package::update(const dovah::loaded_forms::Package& src) {
      bool changed = false;

      auto type = src.type;
      if (type != this->legacy_type) {
         this->legacy_type = type;
         changed = true;
      }

      return changed;
   }
}