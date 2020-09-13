#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class TopicInfo : public Form {
      //
      // Intentionally minimal for now.
      //
      public:
         TopicInfo() : Form(form_type::topic_info) {};

         void load(tes_record_reader&); // TODO: FINISH ME
         static void generateUseInfo(tes_record_reader&, form_stub*);
   };
}