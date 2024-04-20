#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "structs/color_dword.h"

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
         
      public:
         static constexpr const enum form_type form_type = form_type::color;
         Color(const constructor_params& c) : Form(form_type, c) {};

         localized_string name;
         color_t          color;
         color_flags_t    color_flags = 0;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override {}
   };
}