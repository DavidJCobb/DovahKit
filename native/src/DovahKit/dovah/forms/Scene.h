#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/legacy_script.h"
#include "components/papyrus.h"
#include "../data/dialogue/emotion.h"

namespace dovah::loaded_forms {
   class Scene : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::scene;
         Scene(const constructor_params& c) : Form(form_type, c) {};

         struct scene_flag {
            scene_flag() = delete;
            enum {
               begin_on_quest_start = 1 << 0,
               end_on_quest_stop    = 1 << 1,
               // unknown
               loop_while_conditions_are_met = 1 << 3,
               interruptible = 1 << 4,
            };
         };

         struct phase {
            // HNAM
            std::string name; // HNAM+NAM0
            struct {
               components::condition_list start;      // HNAM+...+CTDA[]
               components::condition_list completion; // HNAM+...+NEXT+CTDA[]
            } conditions;
            uint32_t editor_display_width = 32; // HNAM+...+NEXT+...+NEXT+WNAM
            // HNAM+...+HNAM

            bool load(tes_record_reader&, load_order_interfaces::form_load& intfc, size_t which);
            static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
            void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
            void clear(loaded_forms::Form& my_containing_form) noexcept;
            void clone_from(const phase& src, loaded_forms::Form& my_containing_form) noexcept;
            void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept;
         };

         struct actor {
            struct behavior_flag {
               behavior_flag() = delete;
               enum {
                  death_pauses          = 1 << 0, // unused
                  death_ends            = 1 << 1,
                  combat_pauses         = 1 << 2,
                  combat_ends           = 1 << 3,
                  dialogue_pauses       = 1 << 4,
                  dialogue_ends         = 1 << 5,
                  observe_corpse_pauses = 1 << 6,
                  observe_corpse_ends   = 1 << 7,
               };
            };
            struct participation_flag {
               participation_flag() = delete;
               enum {
                  no_player_activation = 1 << 0,
                  optional             = 1 << 1,
               };
            };

            uint32_t alias_id            = 0; // ALID, if not preceded by ANAM
            uint32_t participation_flags = 0; // LNAM
            uint32_t behavior_flags      = 0; // DNAM // controls whether events occurring to this actor pause or end the scene
         };

         enum class action_type : uint16_t {
            dialogue,
            package,
            timer,
         };

         struct action {
            public:
               struct flag {
                  flag() = delete;
                  enum {
                     face_target      = 1 << 17,
                     looping          = 1 << 18,
                     headtrack_player = 1 << 19,
                  };
               };

               struct dialogue_data {
                  form_reference_t topic; // DATA
                  uint32_t headtrack_alias_id = -1; // HTID
                  struct {
                     dialogue::emotion type  = dialogue::emotion::neutral; // DEMO
                     uint32_t          value = 50; // DEVA
                  } emotion;
                  struct {
                     float min = 0.0F; // DMIN
                     float max = 0.0F; // DMAX
                  } looping;
               };
               struct package_data {
                  std::vector<form_reference_t> packages; // PNAM[]
               };
               struct timer_data {
                  float duration = 0.0F; // SNAM // seconds
               };

            public:
               std::variant<
                  dialogue_data,
                  package_data,
                  timer_data
               > data;
               std::string name;           // NAM0
               uint32_t    alias_id  = -1; // ANAM+...+ALID
               uint32_t    action_id = 0; // INAM
               uint32_t    flags     = 0; // FNAM
               struct {
                  uint32_t start = 0; // SNAM
                  uint32_t end   = 0; // ENAM
               } phase_indices;

               constexpr action_type type() const noexcept {
                  if (std::holds_alternative<dialogue_data>(this->data))
                     return action_type::dialogue;
                  if (std::holds_alternative<package_data>(this->data))
                     return action_type::package;
                  if (std::holds_alternative<timer_data>(this->data))
                     return action_type::timer;
                  std::unreachable();
               }

            protected:
               bool _load_base_properties(tes_record_reader&, load_order_interfaces::form_load& intfc);
               bool _load_data_for_dialogue(tes_record_reader&, load_order_interfaces::form_load& intfc);
               bool _load_data_for_package(tes_record_reader&, load_order_interfaces::form_load& intfc);
               bool _load_data_for_timer(tes_record_reader&, load_order_interfaces::form_load& intfc);

            public:
               bool load(tes_record_reader&, load_order_interfaces::form_load& intfc, action_type);
               static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&, action_type);
               void save(tes_record_writer&, load_order_interfaces::form_save& intfc); // NOTE: DOES include the leading ANAM
               void clear(loaded_forms::Form& my_containing_form) noexcept;
               void clone_from(const action& src, loaded_forms::Form& my_containing_form) noexcept;
               void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept;
         };

         components::papyrus_attachment_data script_data; // VMAD
         //
         uint32_t            scene_flags = 0; // FNAM
         std::vector<phase>  phases;  // Each phase begins AND ENDS with an HNAM.
         std::vector<actor>  actors;
         std::vector<action> actions; // Each action begins AND ENDS with an ANAM.
         //
         dialogue_quest_reference_t owning_quest;       // PNAM
         uint32_t                   last_action_id = 0; // INAM
         components::condition_list loop_conditions;

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