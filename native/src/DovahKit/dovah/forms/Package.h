#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/idle_collection.h"
#include "components/papyrus.h"
#include "structs/package_location.h"
#include "structs/package_data_topic.h"
#include "structs/package_event_addon.h"
#include "structs/package_schedule.h"
#include "structs/package_target.h"
#include "../data/package_data_type.h"
#include "../data/packages/interrupt_override_type.h"
#include "../data/packages/legacy_type.h"
#include "../data/packages/preferred_movement_speed.h"
#include "../data/packages/procedure_node_type.h"
#include "../data/package_type.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   class base;
}

namespace dovah::loaded_forms {
   class Package : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::package;
         Package(const constructor_params& c) : Form(form_type, c) {};

         using interrupt_override_type  = packages::interrupt_override_type;
         using legacy_type              = packages::legacy_type;
         using preferred_movement_speed = packages::preferred_movement_speed;

         struct general_flag {
            enum type : uint32_t {
               offers_services          = 1 << 0,
               must_complete            = 1 << 2,
               maintain_speed_at_goal   = 1 << 3,
               treat_as_player_follower = 1 << 4,
               unlock_doors_at_start    = 1 << 6,
               unlock_doors_at_end      = 1 << 7,
               request_block_idles      = 1 << 8,
               continue_if_player_near  = 1 << 9,
               once_per_day             = 1 << 10,
               skip_load_into_furniture = 1 << 12,
               has_preferred_speed      = 1 << 13,
               always_sneak             = 1 << 17,
               allow_swimming           = 1 << 18,
               ignore_combat            = 1 << 20,
               weapons_unequipped       = 1 << 21,
               weapon_drawn             = 1 << 23,
               no_combat_alert          = 1 << 27,
               wear_sleep_outfit        = 1 << 29,
            };
         };
         using general_flags_t = std::underlying_type_t<general_flag::type>;

         struct interrupt_flag {
            enum type : uint16_t {
               hellos_to_player        = 0x0001,
               random_conversations    = 0x0002,
               observe_combat          = 0x0004,
               observe_corpse          = 0x0008,
               react_to_player_actions = 0x0010,
               friendly_fire_comments  = 0x0020,
               aggro_radius_behavior   = 0x0040,
               allow_idle_chatter      = 0x0080,
               //
               world_interactions      = 0x0200,
            };
         };
         using interrupt_flags_t = std::underlying_type_t<interrupt_flag::type>;

         #pragma region Custom package data
            using packdata_unique_id = uint8_t;
            static constexpr const packdata_unique_id no_unique_id = 0xFF;

            using packdata_value = std::variant< // indices should line up with dovah::package_data_type
               bool,    // CNAM // BGSPackageDataBool
               float,   // CNAM // BGSPackageDataFloat
               int32_t, // CNAM // BGSPackageDataInt
               structs::package_location,  // PLDT      // BGSPackageDataLocation
               float,   // CNAM // BGSPackageDataObjectList
               structs::package_location,  // PTDA      // BGSPackageDataRef
               structs::package_location,  // PTDA/PTDT // BGSPackageDataTargetSelector
               structs::package_data_topic // PDTO/TPIC // BGSPackageDataTopic
            >;

            struct packdata_base {
               std::string        name; // BNAM
               packdata_unique_id unique_id = no_unique_id; // UNAM
               bool               is_public = false; // PNAM
               packdata_value     value;

               package_data_type get_type() const noexcept { return (package_data_type)this->value.index(); }
            };

            #pragma region Procedure tree
               struct procedure_flag_overrides {
                  struct {
                     general_flags_t set   = 0;
                     general_flags_t clear = 0;
                  } general;
                  struct {
                     interrupt_flags_t set   = 0;
                     interrupt_flags_t clear = 0;
                  } interrupt;
                  preferred_movement_speed preferred_speed = preferred_movement_speed::run;
               };

               struct procedure_tree_node {
                  package_procedure_node_type type = package_procedure_node_type::procedure;
                  procedure_flag_overrides    flags;

                  std::string name;
                  std::vector<procedure_tree_node*> children;
                  std::vector<packdata_unique_id> parameters; // packdata unique IDs
               };
            #pragma endregion
         #pragma endregion

      public:
         components::condition_list conditions; // CTDA[]
         components::idle_collection idles; // IDLF+IDLC+IDLA+IDLT
         components::papyrus_attachment_data script_data; // VMAD
         //
         general_flags_t           general_flags      = 0; // PKDT+0x00
         legacy_type               type               = legacy_type::custom; // PKDT+0x04
         interrupt_override_type   interrupt_override = interrupt_override_type::none; // PKDT+0x05
         preferred_movement_speed  preferred_speed    = preferred_movement_speed::run; // PKDT+0x06
         interrupt_flags_t         interrupt_flags    = 0; // PKDT+0x08
         uint16_t                  legacy_typed_flags = 0; // PKDT+0x0A
         structs::package_schedule schedule; // PSDT
         form_reference_t combat_style; // CNAM -> CSTY
         form_reference_t owning_quest; // QNAM -> QUST
         structs::typed_package_info::base* typed_info = nullptr;
         union _ {
            ~_() { this->list.~array(); }

            std::array<structs::package_event_addon, 3> list = {};
            struct {
               structs::package_event_addon begin;
               structs::package_event_addon end;
               structs::package_event_addon change;
            };
         } events;

      protected:
         void _force_type_during_load(legacy_type, bool complain_on_change);
      public:
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