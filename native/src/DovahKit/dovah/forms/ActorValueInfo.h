#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"
#include "../data/skill_level.h"

namespace dovah::loaded_forms {
   class ActorValueInfo : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::actor_value_info;
         ActorValueInfo(const constructor_params& c) : Form(form_type, c) {};

         enum class skill_category {
            none,
            combat,
            magic,
            stealth,
         };

         struct perk_tree_node {
            public:
               struct flag {
                  enum type : uint32_t {
                     parents_required = 0x00000001,
                  };
               };

            public:
               uint32_t id = 0; // INAM
               form_reference_t perk; // PNAM // may be null, for the root node of the tree
               uint32_t flags = 0; // FNAM
               uint32_t x = 0; // XNAM
               uint32_t y = 0; // YNAM
               struct {
                  float h = 0; // HNAM // horizontal
                  float v = 0; // VNAM // vertical
               } position;
               form_reference_t skill; // SNAM -> AVIF
               std::vector<uint32_t> connections; // CNAM[] // node IDs
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name        = localized_string(localized_string_type::common);      // FULL
         localized_string description = localized_string(localized_string_type::description); // DESC
         //
         std::string abbreviation; // ANAM
         std::string icon; // ICON
         struct {
            float skill_use_mult       = 1; // AVSK+0x00
            float skill_use_offset     = 0; // AVSK+0x04
            float skill_improve_mult   = 1; // AVSK+0x08
            float skill_improve_offset = 0; // AVSK+0x0C
            skill_category category = skill_category::none; // CNAM
         } skill_info;
         std::vector<perk_tree_node> perk_tree_nodes; // (PNAM...INAM)[]

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