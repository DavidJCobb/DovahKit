#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class FormList : public Form {
      public:
         static constexpr form_type_t form_type = form_type::formlist;
         FormList() : Form(form_type) {};

         std::vector<form_id_t> contents;

         void load(tes_record_reader&);
         static void generateUseInfo(tes_record_reader&, form_stub*);
   };
}