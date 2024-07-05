#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/keyword_list.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class MiscItem : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::misc_item;
         MiscItem(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               non_playable = 0x00000004,
            };
         };

         components::object_bounds bounds; // OBND
         components::model_ts model; // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         std::optional<components::destruction_stage_data> destruction_data; // DEST
         components::keyword_list keywords; // KSIZ, KWDA
         //
         localized_string name; // FULL
         std::string      icon; // ICON
         std::string      message_icon; // MICO
         form_reference_t take_sound; // YNAM // sound when picked up
         form_reference_t drop_sound; // ZNAM // sound when dropped
         float   weight = 0;
         int32_t value  = 0;

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