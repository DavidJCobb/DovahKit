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

         struct body_part_flag {
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
         using body_part_flags_t = std::underlying_type_t<body_part_flag::type>;

      protected:
         struct part_combat_data {
            uint8_t actor_value_id = (uint8_t)-1; // BPND+07
            uint8_t chance_to_hit  = 5;    // BPND+08
            float   damage_mult    = 1.0F; // BPND+00
            uint8_t health_percent = 100;  // BPND+06
         };
         struct part_gore_effect_transform {
            cobb::vector3<float> pos;  // BPND+2C,30,34
            cobb::vector3<float> rot;  // BPND+38,3C,40
         };
         struct part_explodable_limb_replacement {
            components::model model;        // path: NAM1; texture info: NAM5
            float             scale = 1.0F; // BPND+50
         };
         struct part_node_names {
            std::string gore_effect; // NAM4
            std::string ik_start;    // BPNI
            std::string main;        // BPNN
            std::string vats_target; // BPNT
         };

         template<bool UseInfoManaged>
         struct _part_base {
            public:
               using flag    = body_part_flag;
               using flags_t = body_part_flags_t;

               using form_use = std::conditional_t<UseInfoManaged, form_reference_t, form_stub*>;
               
            public:
               localized_string name;       // BPTN
               flags_t          flags = 0;  // BPND+04
               enum limb        limb  = {}; // BPND+05
               part_combat_data combat;
               float headtracking_max_angle = 0.0F; // BPND+14
               struct {
                  part_gore_effect_transform effect_positioning;
                  struct {
                     uint8_t  chance       = 0;    // BPND+09
                     form_use debris;              // BPND+0C
                     uint16_t debris_count = 0;    // BPND+0A
                     float    debris_scale = 1.0F; // BPND+18
                     uint8_t  decal_count  = 0;    // BPND+4D
                     form_use explosion;           // BPND+10
                     form_use impact_data_set;     // BPND+48
                     part_explodable_limb_replacement limb_replacement;
                  } explodable;
                  struct {
                     form_use debris;              // BPND+20
                     int32_t  debris_count = 0;    // BPND+1C
                     float    debris_scale = 1.0F; // BPND+28
                     uint8_t  decal_count  = 0;    // BPND+4C
                     form_use explosion;           // BPND+24
                     form_use impact_data_set;     // BPND+44
                  } severable;
               } gore;
               part_node_names nodes;
               std::string     pose_matching; // PNAM // *.psa file path, relative to but not including "Data\Meshes\"

            public:
               constexpr bool empty() const noexcept {
                  return this->nodes.part.empty();
               }
         };

      public:
         using unmanaged_part = _part_base<false>;
         struct part : public _part_base<true> {
            public:
               // First subrecord must already have been opened
               bool load(tes_record_reader&, load_order_interfaces::form_load& intfc);
               void save(tes_record_writer& record, load_order_interfaces::form_save& intfc);
               void clone_from(BodyPartData& my_owner, const part& src);
               void clear(BodyPartData& owner);
               bool sever_outbound_references_to(BodyPartData& owner, form_stub& other) noexcept; // returns true if any changes made
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
         std::vector<part> parts;
         form_reference_t ragdoll; // RAGA -> BGSRagdoll

         std::string base_node_name() const;

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