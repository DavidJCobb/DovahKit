#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class Color : public Form {
      public:
         struct color_flag {
            color_flag() = delete;
            enum type : uint32_t {
               playable = 0x01
            };
         };
         using color_flags_t = std::underlying_type_t<color_flag::type>;
         //
      public:
         static constexpr form_type_t form_type = form_type::color;
         Color() : Form(form_type) {};

         localized_string name;
         union {
            struct {
               uint8_t r;
               uint8_t g;
               uint8_t b;
               uint8_t unused;
            };
            uint32_t hex = 0;
         } color;
         color_flags_t color_flags = 0;

         void load(tes_record_reader&);
         static void generateUseInfo(tes_record_reader&, form_stub*);
         //
      protected:
         virtual bool _save_impl(tes_file_writing::record& record) override;
   };
}