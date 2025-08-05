#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Climate : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::climate;
         Climate(const constructor_params& c) : Form(form_type, c) {};

         enum class moons {
            none    = 0,
            masser  = 1,
            secunda = 2,
            both    = 3,
         };

         struct moonstamp {
            uint8_t value = 0;

            constexpr void join(moons m, uint8_t phase_length) {
               this->value = ((uint8_t)m << 6) | (phase_length % 64);
            }
            constexpr void split(moons& m, uint8_t& phase_length) const {
               m            = (moons)((this->value >> 6) & 0b11);
               phase_length = this->value % 64;
            }
         };
         struct timestamp {
            uint8_t value = 0;

            constexpr void join(uint8_t h, uint8_t m) {
               this->value = (h * 6) + (m / 10);
            }
            constexpr void split(uint8_t& h, uint8_t& m) const {
               h = this->value / 6;
               m = (this->value % 6) * 10;
            }
         };
         struct weather_type {
            form_reference_t weather;
            int32_t          chance = 0;
            form_reference_t global;
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         components::model night_sky_nif; // MODL, MODT
         struct {
            std::string sun;
            std::string sun_glare;
         } textures;
         struct {
            struct {
               timestamp begin;
               timestamp end;
            } sunrise;
            struct {
               timestamp begin;
               timestamp end;
            } sunset;
            uint8_t   volatility = 0;
            moonstamp moon_phase;
         } timing;
         std::vector<weather_type> weather_types;

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