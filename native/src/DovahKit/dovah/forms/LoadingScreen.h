#pragma once
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/papyrus.h"
#include "helpers/vector3.h"

namespace dovah::loaded_forms {
   class LoadingScreen : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::loading_screen;
         LoadingScreen(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               displays_in_main_menu = 1 << 10,
            };
         };

      public:
         components::condition_list conditions; // CTDA and friends
         components::papyrus_attachment_data script_data;
         //
         localized_string description = localized_string(localized_string_type::common); // DESC
         //
         std::string camera_path; // MOD2 // file path
         struct {
            cobb::vector3<int16_t> rotation    = {}; // RNAM // degrees
            cobb::vector3<float>   translation = {}; // XNAM
            float scale = 1; // SNAM
         } initial_coords;
         struct {
            int16_t min = -180; // degrees
            int16_t max =  180; // degrees
         } rotation_constraints;
         form_reference_t static_model; // NNAM -> STAT

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}