#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Note : public Form {
      public:
         static constexpr form_type_t form_type = form_type::note;
         Note(const constructor_params& c) : Form(form_type, c) {};

         enum note_type : uint8_t {
            sound = 0,
            text  = 1,
            image = 2,
            voice = 3
         };

         components::object_bounds bounds; // OBND
         components::model model; // MODL, MODT
         components::papyrus_attachment_data script_data; // VMAD
         //
         note_type type = note_type::text;
         //
         localized_string name; // FULL
         std::string      icon; // ICON
         form_reference_t take_sound; // YNAM // sound when picked up
         form_reference_t drop_sound; // ZNAM // sound when dropped
         std::vector<form_reference_t> owning_quests; // ONAM
         struct {
            form_reference_t sound;   // SNAM // only for note_type::sound
            form_reference_t speaker; // SNAM // only for note_type::voice
            localized_string text;    // TNAM // only for note_type::text
            std::string      image;   // XNAM // only for note_type::image
            form_reference_t topic;   // TNAM // only for note_type::voice
         } content;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}