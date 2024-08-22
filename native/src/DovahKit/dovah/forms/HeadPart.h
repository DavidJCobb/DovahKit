#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "../data/headparts.h"

namespace dovah::loaded_forms {
   class HeadPart : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::head_part;
         HeadPart(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               non_playable = 0x00000004,
            };
         };
         
         struct head_part_flag {
            head_part_flag() = delete;
            enum type : uint8_t {
               playable       = 0x01,
               male           = 0x02,
               female         = 0x04,
               is_extra_part  = 0x10, // e.g. scars, etc.
               use_solid_tint = 0x20,
            };
         };
         using head_part_flags_t = std::underlying_type_t<head_part_flag::type>;

         components::model_ts model; // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string  name; // FULL
         head_part_type    type = head_part_type::misc;
         head_part_flags_t flags = 0;
         form_reference_t  color;       // CNAM // color form
         form_reference_t  texture_set; // TNAM // texture set
         form_reference_t  valid_races; // RNAM // form list of races
         struct {
            std::string race;
            std::string tri;
            std::string chargen;
         } morphs;
         std::vector<form_reference_t> extra_parts; // HNAM, one per // other head parts

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}