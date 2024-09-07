#pragma once
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Grass : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::grass;
         Grass(const constructor_params& c) : Form(form_type, c) {};

         enum class units_from_water_comparator : uint32_t {
            above_at_least,
            above_at_most,
            below_at_least,
            below_at_most,
            either_at_least,
            either_at_most,
            either_at_most_above,
            either_at_most_below,
         };

         struct grass_flag {
            grass_flag() = delete;
            enum type : uint8_t {
               vertex_lighting = 0x01,
               uniform_scaling = 0x02,
               fit_to_slope    = 0x04,
            };
         };
         using grass_flags_t = std::underlying_type_t<grass_flag::type>;

         components::object_bounds bounds; // OBND
         components::model_ts model; // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         //
         uint8_t density = 0;
         struct {
            uint8_t min = 0;
            uint8_t max = 0;
         } slope;
         struct {
            units_from_water_comparator comparator = units_from_water_comparator::above_at_least;
            uint16_t units = 0;
         } distance_from_water;
         float position_range = 0;
         float height_range = 0;
         float color_range = 0;
         float wave_period = 0;
         grass_flags_t grass_flags = 0;

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