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
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override {}
   };
}