#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class TopicInfo : public Form {
      //
      // Intentionally minimal for now.
      //
      public:
         static constexpr form_type_t form_type = form_type::topic_info;
         TopicInfo() : Form(form_type) {};

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc); // TODO: FINISH ME
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
   };
}