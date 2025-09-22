#include "./topic.h"
#include "dovah/data/dialogue/topic_subtype.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/Topic.h"
#include "dovah/core.h"

namespace {
   using loaded_form_type = dovah::loaded_forms::Topic;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   void topic::skim_record(dovah::tes_file_reading::record& record) {
      struct {
         loaded_form_type::record_skimmers::subtype subtype;
      } skimmers;

      while (auto& subrecord = record.next_subrecord()) {
         skimmers.subtype.skim_subrecord(subrecord);
      }

      skimmers.subtype.finalize();
      if (skimmers.subtype.result.has_value()) {
         this->subtype_signature = skimmers.subtype.result.value();
      }
   }
   bool topic::update(const dovah::loaded_forms::Topic& src) {
      auto subtype = src.subtype;
      if (subtype == 0) {
         auto i = src.data.subtype;
         if (i < dovah::dialogue::all_topic_subtypes.size())
            subtype = dovah::dialogue::all_topic_subtypes[i].signature;
      }

      bool changed = false;
      if (this->subtype_signature != subtype)
         changed = true;
      
      this->subtype_signature = subtype;

      return changed;
   }
}