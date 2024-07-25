#pragma once
#include <bitset>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "helpers/bitset.h"
#include "helpers/vector3.h"
#include "Form.h"
#include "_common.h"
#include "components/attack_data.h"
#include "components/biped_object.h"
#include "components/keyword_list.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "components/spell_list.h"
#include "../data/face_fx/phonemes.h"
#include "../data/skills.h"
#include "../utils/data_by_sex.h"

namespace dovah::loaded_forms {
   class Race : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::race;
         Race(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const size_t max_biped_object_name_count  = 32;
         static constexpr const size_t max_biped_object_name_length = 0x103; // MAX_PATH - 1

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               critter = 0x00080000, // guessed
            };
         };

         struct race_flag {
            race_flag() = delete;
            enum type : uint8_t {
               playable                        = 0x00000001,
               facegen_head                    = 0x00000002,
               child                           = 0x00000004,
               tilt_front_back                 = 0x00000008,
               tilt_left_right                 = 0x00000010,
               no_shadow                       = 0x00000020,
               swims                           = 0x00000040,
               flies                           = 0x00000080,
               walks                           = 0x00000100,
               immobile                        = 0x00000200,
               not_pushable                    = 0x00000400,
               no_water_combat                 = 0x00000800,
               no_rotate_to_headtrack          = 0x00001000,
               dont_show_blood_spray           = 0x00002000,
               dont_show_blood_decal           = 0x00004000,
               uses_headtrack_anims            = 0x00008000,
               spells_align_with_magic_node    = 0x00010000,
               use_world_raycasts_for_foot_ik  = 0x00020000,
               allow_ragdoll_collision         = 0x00040000,
               regen_health_in_combat          = 0x00080000,
               cant_open_doors                 = 0x00100000,
               allow_player_dialogue           = 0x00200000,
               no_knockdowns                   = 0x00400000,
               can_be_pickpocketed             = 0x00800000,
               always_use_proxy_controller     = 0x01000000,
               dont_show_weapon_blood          = 0x02000000,
               overlay_head_part_list          = 0x04000000, // mutually exclusive with next
               override_head_part_list         = 0x08000000, // mutually exclusive with previous
               can_pick_up_items               = 0x10000000,
               allow_multiple_membrane_shaders = 0x20000000,
               can_dual_wield                  = 0x40000000,
               avoids_roads                    = 0x80000000,
            };
         };
         using race_flags_t = std::underlying_type_t<race_flag::type>;

         struct alt_flag {
            enum type : uint32_t {
               use_advanced_avoidance = 0x00000001,
               non_hostile            = 0x00000002,
               allow_mounted_combat   = 0x00000010,
            };
         };
         using alt_flags_t = std::underlying_type_t<alt_flag::type>;

         // Flags indicating what weapons (and weapon-like items) actors of this race are allowed to equip.
         struct equipment_flag {
            enum type : uint32_t {
               hand_to_hand_melee = 0x00000001,
               one_hand_sword     = 0x00000002,
               one_hand_dagger    = 0x00000004,
               one_hand_axe       = 0x00000008,
               one_hand_mace      = 0x00000010,
               two_hand_sword     = 0x00000020,
               two_hand_axe       = 0x00000040,
               bow                = 0x00000080,
               staff              = 0x00000100,
               spell              = 0x00000200,
               shield             = 0x00000400,
               torch              = 0x00000800,
               crossbow           = 0x00001000,
            };
         };
         using equipment_flags_t = std::underlying_type_t<equipment_flag::type>;

         enum class creature_size : uint32_t {
            small       = 0,
            medium      = 1,
            large       = 2,
            extra_large = 3,
         };

         struct movement_type_override {
            form_reference_t type; // MTYP
            union {
               struct {
                  float left_walk;
                  float left_run;
                  float right_walk;
                  float right_run;
                  float forward_walk;
                  float forward_run;
                  float back_walk;
                  float back_run;
                  float rotate_walk;
                  float rotate_run;
                  float unknown;
               };
               std::array<float, 11> list = {}; // SPED
            } values;
         };

         enum class tint_type : uint16_t {
            none,
            lip_color,
            cheek_color_upper,
            eyeliner,
            eyeshadow_upper,
            eyeshadow_lower,
            skin_tone,
            facepaint,
            laugh_lines,
            cheek_color_lower,
            nose,
            chin,
            neck,
            forehead,
            dirt,
            unknown_16,
         };

         struct tint_preset {
            form_reference_t color;     // TINC // type is CLFM
            int16_t          index = 0; // TIRS
            float            alpha = 1; // TINV
         };
         //
         struct face_tint {
            int32_t          index;         // TINI // instantiates the tint.
            form_reference_t default_color; // TIND // type is CLFM
            tint_type        type = tint_type::none; // TINP
            std::string      texture;       // TINT // DDS path
            std::vector<tint_preset> presets;
         };

         struct skill_boost {
            dovah::skill skill; // actor value enum
            uint8_t      boost = 0;
         };

         struct sex_data {
            // Field                                 // Male      // Female
            components::model    behavior_graph;     // NAM3 ~ MODL+MODT       // *.hkx file
            form_reference_t     decapitate_armor;   // DNAM+0x00 // DNAM+0x04
            struct {
               form_reference_t              default_face_texture; // type is TXST
               form_reference_t              default_hair_color; // HCLF
               std::vector<form_reference_t> face_textures; // FTSM[] and FTSF[] // type is TXST
               std::vector<face_tint>        face_tints;
               std::vector<form_reference_t> hair_colors; // AHCF[] // type is CLFM
               std::vector<form_reference_t> head_parts;  // NAM0 ~ HEAD[]; or INAM[]; or JNAM[]
               struct {
                  cobb::bitset<256> brows;  // MPAI{1}+MPAV
                  cobb::bitset<256> eyes;   // MPAI{2}+MPAV
                  cobb::bitset<256> mouths; // MPAI{3}+MPAV
                  cobb::bitset<256> noses;  // MPAI{0}+MPAV
               } morphs;
               std::vector<form_reference_t> preset_actors; // RPRF[] // type is NPC_
            } head_data;
            float                height_mult = 1;    // DATA+0x10 // DATA+0x14
            components::model    lighting_model;     // NAM1 ~ MODL+MODT       // *.egt file; unused in Skyrim
            components::model    skeleton_nif;       // ANAM
            form_reference_t     voicetype;          // VTCK+0x00 // VTCK+0x04
            float                weight      = 1;    // DATA+0x18 // DATA+0x1C
         };

         components::attack_data             attack_data;  // ATKR, ATKD, ADKE
         components::biped_object            biped_object; // BOD2 or older BODT
         components::papyrus_attachment_data script_data;  // VMAD
         components::keyword_list            keywords;     // KSIZ, KWDA
         components::spell_list              spells;       // SPCT, SPLO
         //
         localized_string name;        // FULL
         localized_string description; // DESC
         //
         race_flags_t      race_flags          = 0; // DATA+0x20
         alt_flags_t       alt_flags           = 0; // DATA+0x7C
         form_reference_t  armor_race          = nullptr; // RNAM // type is RACE. overrides what armors this race can wear
         form_reference_t  body_part_data      = nullptr; // GNAM // type is BPTD
         form_reference_t  decapitation_effect = nullptr; // NAM7 // type is ARTO
         form_reference_t  impact_data_set     = nullptr; // NAM5 // type is IPDS
         form_reference_t  material_type       = nullptr; // NAM4 // type is MATT
         form_reference_t  morph_race          = nullptr; // NAM8 // type is RACE
         form_reference_t  skin                = nullptr; // WNAM // type is ARMO
         //
         uint16_t tint_layer_count = 0; // TINL
         //
         struct {
            int32_t body   = -1; // DATA+0x68
            int32_t hair   = -1; // DATA+0x48
            int32_t head   = -1; // DATA+0x44
            int32_t shield = -1; // DATA+0x50
            std::array<std::string, max_biped_object_name_count> names;
         } biped_object_info;
         data_by_sex<sex_data> by_sex;
         struct {
            form_reference_t open  = nullptr; // ONAM // open loot
            form_reference_t close = nullptr; // LNAM // close loot
         } container_sounds;
         struct {
            equipment_flags_t flags = 0xFFFFFFFF; // VNAM
            std::vector<form_reference_t> equip_slots; // QNAM[] // type is EQUP
            form_reference_t  unarmed_equip_slot = nullptr; // UNES
         } equipment;
         struct {
            float face = 0; // UNAM
            float main = 0; // PNAM
         } facegen_clamp;
         struct {
            float acceleration_rate = 0.25; // DATA+0x38
            float deceleration_rate = 0; // DATA+0x3C
            //
            float angular_acceleration_rate = 0; // DATA+0x74
            float angular_tolerance = 0; // DATA+0x78

            union {
               struct {
                  form_reference_t walk;   // WKMV
                  form_reference_t run;    // RNMV
                  form_reference_t swim;   // SWMV
                  form_reference_t fly;    // FLMV
                  form_reference_t sneak;  // SNMV
                  form_reference_t sprint; // SPMV
               };
               std::array<form_reference_t, 6> list = {};
            } types;
            std::vector<movement_type_override> overrides; // MTYP+SPED
         } movement;
         struct {
            std::vector<std::string> morph_names; // PHNT
            std::array<std::vector<float>, dovah::face_fx::phoneme_count> weights; // PHWT

            // The `weights` list is a list of morph weights by phoneme (i.e. weights[(size_t)phoneme][morph_index]). 
            // The number of morphs in each weight list should match the number of morph names. (In turn, this means 
            // the morph names should appear in the record before the morph weights.)
            //
            // The weights seem to only be loaded by the CK, likely because they're only used for generating LIP files.
         } phonemes;
         struct {
            float aim_angle_tolerance = 0; // DATA+0x6C
            struct {
               float health  = 0;
               float magicka = 0;
               float stamina = 0;
            } attribute_base; // DATA+0x24
            struct {
               float health  = 0;
               float magicka = 0;
               float stamina = 0;
            } attribute_regen; // DATA+0x54
            float base_carry_capacity = 0; // DATA+0x30
            float base_mass = 0; // DATA+0x34
            enum creature_size creature_size = creature_size::medium; // DATA+0x40
            float injured_health_threshold = 0; // DATA+0x4C // in the range [0, 1]
            std::array<std::optional<skill_boost>, 7> skill_boosts; // DATA+0x00
            uint16_t unk0E = 0; // DATA+0x0E
            uint32_t unk70 = 0; // DATA+0x70
            struct {
               float damage = 0; // DATA+0x60
               float reach  = 0; // DATA+0x64
            } unarmed;
         } stats; // DATA
         //
         // Added in a patch:
         //
         struct {
            cobb::vector3<float> climb_on_offset = { -63.479, 0, 0 }; // move actors to this offset when they climb onto this race
            cobb::vector3<float> dismount_offset = { -50, 0, 65 };
            cobb::vector3<float> camera_offset   = { 0, -300, 0 };
         } mount_data; // DATA+0x80

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