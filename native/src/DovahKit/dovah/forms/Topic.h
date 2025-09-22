#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "../data/dialogue/category.h"
#include "components/conditions.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Topic : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::topic;
         Topic(const constructor_params& c) : Form(form_type, c) {};

         struct dialogue_flag {
            dialogue_flag() = delete;
            enum type : uint8_t {
               do_all_before_repeating = 0x01,
            };
         };
         using dialogue_flags_t = std::underlying_type_t<dialogue_flag::type>;

         using category = dialogue::category;

         //
         // Subtype indices aren't actually reliable because they're sequentially numbered, but Bethesda 
         // added six values into the middle of the list for the Dragonborn DLC's flying mount feature. 
         // This means that values at or after the point where those were added are unreliable; for 
         // example, Skyrim.esm has tons of topics that now have the right subtype signature but the 
         // wrong subtype index.
         //
         using subtype_index = uint16_t;

         struct {
            dialogue_branch_reference_t branch; // BNAM
            dialogue_quest_reference_t  quest;  // QNAM
         } owning_forms;
         localized_string text; // FULL // player's dialogue
         struct {
            dialogue_flags_t flags    = 0;
            category         category = category::topic; // overwritten by SNAM, which is generally more reliable
            subtype_index    subtype  = 0;               // overwritten by SNAM, which is generally more reliable
         } data; // DATA
         float    priority = 50.0F;  // PNAM
         uint32_t subtype  = 'CUST'; // SNAM // in-game, the game uses whichever subtype between DATA and SNAM was loaded last
         components::papyrus_attachment_data script_data; // VMAD

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;

      public:
         struct record_skimmers { // namespace
            record_skimmers() = delete;

            class subtype {
               protected:
                  bool     has_snam  = false;
                  uint8_t  index     = 0;
                  uint32_t signature = 0;
               public:
                  std::optional<uint32_t> result;

                  void skim_subrecord(tes_subrecord_reader&);
                  void finalize();
            };
         };
   };
}