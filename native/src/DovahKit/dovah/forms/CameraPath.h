#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class CameraPath : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::camera_path;
         CameraPath(const constructor_params& c) : Form(form_type, c) {};

         enum class zoom_type {
            default_,
            disable,
            shot_list,
         };

      public:
         components::condition_list conditions;
         components::papyrus_attachment_data script_data; // VMAD
         //
         form_reference_t parent;           // ANAM+0x00 -> CPTH
         form_reference_t previous_sibling; // ANAM+0x04 -> CPTH
         struct {
            zoom_type type = zoom_type::default_;
            bool must_have_camera_shots = true;
         } zoom;
         std::vector<form_reference_t> camera_shots; // SNAM[] -> CAMS

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}