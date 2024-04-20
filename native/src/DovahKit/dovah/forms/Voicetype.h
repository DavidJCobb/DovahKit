#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class Voicetype : public Form {
      public:
         struct voicetype_flag {
            voicetype_flag() = delete;
            enum type : uint8_t {
               allow_default_dialogue = 0x01,
               female                 = 0x02,
            };
         };
         using voicetype_flags_t = std::underlying_type_t<voicetype_flag::type>;
         //
      public:
         static constexpr const enum form_type form_type = form_type::voicetype;
         Voicetype(const constructor_params& c) : Form(form_type, c) {};

         voicetype_flags_t voicetype_flags = 0;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override {}
         virtual void _clear_impl() noexcept override;
   };
}