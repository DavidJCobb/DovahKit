#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class ShaderParticleGeometry : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::shader_particle_geometry_data;
         ShaderParticleGeometry(const constructor_params& c) : Form(form_type, c) {};

         enum class particle_type : uint32_t {
            rain = 0,
            snow = 1,
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         uint32_t box_size = 4096; // minimum allowed value is 1
         struct {
            float min = 0;
            float max = 0;
         } center_offset;
         float gravity_velocity = 0;
         struct {
            float density = 1; // minimum allowed value is 1
            struct {
               float x = 1;
               float y = 1;
            } size;
            std::string texture;
         } particles;
         struct {
            float initial_range = 0;
            float velocity      = 0;
         } rotation;
         struct {
            uint32_t x = 1; // minimum allowed value is 1
            uint32_t y = 1; // minimum allowed value is 1
         } subtexture_count;
         particle_type type = particle_type::rain;

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