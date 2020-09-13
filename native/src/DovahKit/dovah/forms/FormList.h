#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class FormList : public Form {
      public:
         FormList() : Form(form_type::formlist) {};

         std::vector<form_id_t> contents;

         void load(tes_record_reader&);
         static void generateUseInfo(tes_record_reader&, form_stub*);
   };
}