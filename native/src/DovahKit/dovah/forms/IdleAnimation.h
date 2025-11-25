#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class IdleAnimation : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::idle;
         IdleAnimation(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const uint8_t loop_time_forever = 0xFF;

         static constexpr const size_t max_event_name_length = 0x104;
         static constexpr const size_t max_filename_length   = 0x104;

         struct flag {
            enum type : uint8_t {
               parent       = 0x01,
               sequence     = 0x02,
               no_attacking = 0x04,
               blocking     = 0x08,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

         struct action_root_candidacy {
            std::string      behavior_graph_path;
            form_reference_t action;
            struct {
               const tes_file_reading::file_loader* source_file = nullptr;
               struct {
                  size_t of_record    = 0; // record offset within containing file
                  size_t of_subrecord = 0; // subrecord offset within record body (in case record was compressed and had multiple ANAM))
               } offsets;
            } anam_subrecord;
         };

      public:
         components::condition_list conditions; // CTDA[]
         components::papyrus_attachment_data script_data; // VMAD
         //
         std::string animation_event; // ENAM
         struct {
            struct {
               uint8_t min = 0; // DATA+0x00
               uint8_t max = 0; // DATA+0x01
            } loop_time_range;
            flags_t  flags              = 0; // DATA+0x02
            uint8_t  anim_group_section = 0; // DATA+0x03
            uint16_t replay_delay       = 0; // DATA+0x04
         } data; // DATA
      protected:
         struct {
            std::vector<action_root_candidacy> masters;
            std::vector<action_root_candidacy> active;
         } _as_action_root;
         struct {
            struct {
               std::string corrected;
               std::string verbatim;  // DNAM // HKX file
            } behavior_graph;
            form_reference_t parent;           // ANAM+0x00 // IDLE or AACT
            form_reference_t previous_sibling; // ANAM+0x04 // IDLE
         } _hierarchy;

      public:
         constexpr bool loops_forever() const noexcept {
            return data.loop_time_range.min == loop_time_forever && data.loop_time_range.max == loop_time_forever;
         }

         [[nodiscard]] static std::string correct_behavior_graph_path(std::string_view);

         constexpr const std::string& get_behavior_graph_path(bool verbatim = true) const noexcept;
         void set_behavior_graph_path(std::string_view, bool do_corrections = false);

         constexpr form_stub* get_hierarchy_parent() const noexcept;
         constexpr form_stub* get_hierarchy_previous_sibling() const noexcept;

         void make_action_root(std::string_view behavior_graph, form_stub& action);
         void make_loose();
         void make_loose(std::string_view behavior_graph);
         void make_child(std::string_view behavior_graph, form_stub& parent_idle, form_stub* previous_sibling_idle);

         [[nodiscard]] std::vector<const action_root_candidacy*> get_candidicacies_for_action_root(std::string_view behavior_graph, form_stub& action) const;
         void for_each_action_root_candidacy(std::function<void(const std::string_view, form_stub* action)>);
         bool is_better_action_root_candidate_than(const IdleAnimation& other_idle, const std::string_view graph, form_stub& action) const;

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

#include "./IdleAnimation.inl"