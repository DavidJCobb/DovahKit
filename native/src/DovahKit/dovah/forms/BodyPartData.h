#pragma once
#include <cstdint>
#include "Form.h"
#include "helpers/vector3.h"
#include "_common.h"
#include "components/model.h"
#include "../data/limbs.h"

namespace dovah::loaded_forms {
   class BodyPartData : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::body_part_data;
         BodyPartData(const constructor_params& c) : Form(form_type, c) {};

         struct part {
            public:
               struct flag {
                  enum type : uint8_t {
                     severable = 1 << 0,
                     has_ik_data = 1 << 1,
                     ik_data_biped_data = 1 << 2,
                     explodable = 1 << 3,
                     ik_data_is_head = 1 << 4,
                     ik_data_headtracking = 1 << 5,
                     absolute_explode_chance = 1 << 6,
                  };
               };
               using flags_t = std::underlying_type_t<flag::type>;

            public:
               localized_string name;       // BPTN
               flags_t          flags = 0;  // BPND+04
               enum limb        limb  = {}; // BPND+05
               struct {
                  int8_t  actor_value_id = -1;   // BPND+07
                  uint8_t chance_to_hit  = 5;    // BPND+08
                  float   damage_mult    = 1.0F; // BPND+00
                  uint8_t health_percent = 100;  // BPND+06
               } combat;
               float headtracking_max_angle = 0.0F; // BPND+14
               struct {
                  struct {
                     cobb::vector3<float> pos;  // BPND+2C,30,34
                     cobb::vector3<float> rot;  // BPND+38,3C,40
                  } effect_positioning;
                  struct {
                     uint8_t          chance       = 0;    // BPND+09
                     form_reference_t debris;              // BPND+0C
                     uint16_t         debris_count = 0;    // BPND+0A
                     float            debris_scale = 1.0F; // BPND+18
                     uint8_t          decal_count  = 0;    // BPND+4D
                     form_reference_t explosion;           // BPND+10
                     form_reference_t impact_data_set;     // BPND+48
                     struct {
                        components::model model;        // path: NAM1; texture info: NAM5
                        float             scale = 1.0F; // BPND+50
                     } limb_replacement;
                  } explodable;
                  struct {
                     form_reference_t debris;              // BPND+20
                     int32_t          debris_count = 0;    // BPND+1C
                     float            debris_scale = 1.0F; // BPND+28
                     uint8_t          decal_count  = 0;    // BPND+4C
                     form_reference_t explosion;           // BPND+24
                     form_reference_t impact_data_set;     // BPND+44
                  } severable;
               } gore;
               struct {
                  std::string gore_effect; // NAM4
                  std::string ik_start;    // BPNI
                  std::string part;        // BPNN
                  std::string vats_target; // BPNT
               } nodes;
               std::string pose_matching; // PNAM

            public:
               // First subrecord must already have been opened
               bool load(tes_record_reader&, load_order_interfaces::form_load& intfc);
               void save(tes_record_writer& record, load_order_interfaces::form_save& intfc);
               void clone_from(BodyPartData& my_owner, const part& src);
               void clear(BodyPartData& owner);
               void sever_outbound_references_to(BodyPartData& owner, form_stub& other) noexcept;
         };

      protected:
         struct part_use_info {
            enum limb limb = {}; // BPND+05
            struct {
               struct {
                  form_id_t debris;
                  form_id_t explosion;
                  form_id_t impact_data_set;
               } explodable;
               struct {
                  form_id_t debris;
                  form_id_t explosion;
                  form_id_t impact_data_set;
               } severable;
            } gore;

            bool load(tes_record_reader&);
         };

      public:
         components::model model; // skeleton
         std::array<part, limbs_count> parts; // parts[limb::foo] == part{}
         form_reference_t ragdoll; // RAGA -> BGSRagdoll

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