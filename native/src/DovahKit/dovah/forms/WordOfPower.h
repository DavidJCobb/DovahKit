#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class WordOfPower : public Form {
      public:
         static constexpr form_type_t form_type = form_type::word_of_power;
         WordOfPower() : Form(form_type) {};

         localized_string dragon_name;
         localized_string human_name;

         void load(tes_record_reader&);
         static void generateUseInfo(tes_record_reader&, form_stub*);
         //
      protected:
         virtual bool _save_impl(tes_file_writing::record& record) override;
   };
}