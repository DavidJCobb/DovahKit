#pragma once
#include <cstdint>
#include <string>
#include <optional>
#include <variant>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class MusicTrack : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::music_track;
         MusicTrack(const constructor_params& c) : Form(form_type, c) {};

         struct loop_data {
            float    begin = 0;
            float    end   = 0;
            uint32_t count = 0;
         };

         struct palette_data {
            float duration = 0; // FLTV
            float fade_out = 0; // DNAM
            std::vector<form_reference_t> tracks;     // SNAM[] -> MUST[]
         };
         struct single_data {
            struct {
               std::string main;   // ANAM
               std::string finale; // BNAM
            } filenames;
            std::vector<float> cue_points; // FNAM[]
            std::optional<loop_data> loop; // LNAM
         };
         struct silent_data {
            float duration = 0; // FLTV
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         std::variant<
            palette_data,
            single_data,
            silent_data
         > data;
         components::condition_list conditions; // CITC+CTDA[]

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