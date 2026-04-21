#pragma once
#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "../data/dialogue/emotion.h"
#include "components/conditions.h"
#include "components/legacy_script.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class TopicInfo : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::topic_info;
         TopicInfo(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const size_t max_available_response_ids = std::numeric_limits<uint8_t>::max();

         #pragma region Enums and flags masks
         struct info_flag {
            info_flag() = delete;
            enum type : uint16_t {
               goodbye                     = 0x0001,
               random                      = 0x0002,
               say_once                    = 0x0004,
               random_end                  = 0x0020,
               invisible_continue          = 0x0040,
               walk_away                   = 0x0080,
               walk_away_invisible_in_menu = 0x0100,
               force_subtitle              = 0x0200,
               can_move_while_greeting     = 0x0400,
               no_lip_file                 = 0x0800,
               requires_post_processing    = 0x1000,
               audio_output_override       = 0x2000,
               spends_favor_points         = 0x4000,
            };
         };
         using info_flags_t = std::underlying_type_t<info_flag::type>;

         enum class favor_level_t : uint8_t {
            none   = 0,
            small  = 1,
            medium = 2,
            large  = 3,
         };

         struct load_flag {
            //
            // Flags indicating whether certain subrecords were present.
            //
            load_flag() = delete;
            enum type {
               has_topic_text_override = 0x0001, // was RNAM present?
            };
         };
         using load_flags_t = std::underlying_type_t<load_flag::type>;
         #pragma endregion

         struct response {
            struct flag {
               flag() = delete;
               enum type : uint8_t {
                  use_emotion_animation = 0x01,
               };
            };
            using flags_t = std::underlying_type_t<flag::type>;
            
            struct {
               dovah::dialogue::emotion type = dovah::dialogue::emotion::neutral;
               int32_t value = 50;
            } emotion;
            uint32_t unused;
            uint8_t  id = 0; // Should not be 0. The game and CK both use "New Response" as part of the filename if you specify 0.
            // 3 padding bytes here
            form_reference_t sound; // plays instead of any normal voice line, if set
            flags_t flags = 0;
            // 3 padding bytes here
            localized_string text         = localized_string(localized_string_type::info);
            std::string      script_notes;
            std::string      edits;
            struct {
               form_reference_t speaker;
               form_reference_t listener;
            } idles;

            void clear(TopicInfo& owner);
            void clone_from(const response& other, loaded_forms::Form& my_owner);
            void save(tes_file_writing::record& record, load_order_interfaces::form_save& intfc);
            void sever_outbound_references_to(TopicInfo& owner, form_stub& target);
         };

         info_flags_t  info_flags  = 0;
         load_flags_t  load_flags  = 0;
         favor_level_t favor_level = favor_level_t::none; // CNAM
         uint16_t      raw_hours_until_reset = 0; // range is [0x0000, 0xFFFF], normalized to [0, 24]
         form_reference_t speaker; // ANAM
         form_reference_t topic; // TPIC // unknown. loaded by the CK but not the game. defaults to parent form if missing/null.
         form_reference_t walk_away_topic; // TWAT (yes, really)
         form_reference_t use_shared_info; // DNAM // a SharedInfo to borrow response data from
         form_reference_t audio_output_override; // ONAM
         struct {
            //
            // A TopicInfo's data consists of everything from the winning record, plus anything 
            // other than response data that has been provided by any partial override(s) that 
            // loaded immediately before the winning record. The winning record will overwrite 
            // single fields set by the previous partial-flagged overrides, but will add to any 
            // (non-response) lists provided by the partial-flagged overrides. Consider:
            //
            //    FILE | PARTIAL?
            //    A    | Can't be
            //    B    | Yes
            //    C    | No
            //    D    | Yes
            //    E    | Yes
            //    F    | Doesn't matter
            //
            // The final loaded record will consist of data from files D, E, and F, and will 
            // include all link-to topics and conditions supplied by each of those files.
            //
            // Because the winning record can only append to the link-to and condition lists 
            // in this situation, the list items supplied by the prior partial records cannot 
            // be edited, reordered, deleted, or otherwise altered. Thus, they go into the 
            // "locked" sub-list, while everything else goes into the "normal" sub-list. When 
            // reading the form contents, the "locked" list items come first and the "normal" 
            // list items come later.
            //
            // Modifying the "locked" lists is undefined behavior, as is taking any operation 
            // that leads to their being modified. The "clear" and "sever outbound references" 
            // functions will affect these lists (in the latter case, to avoid dangling refs), 
            // but these functions should never end up actually doing anything to these lists 
            // unless something else has gone wrong (i.e. you shouldn't be able to delete 
            // forms from a master of the active file, which is what would be necessary in 
            // order for a reference from locked data to be severed).
            //
            // The "clone" function has special-case behavior for committing a working copy: 
            // if the clone-to form and the clone-from form have the same stub, and if the 
            // clone-from form is a working copy, then we copy data from each list to its 
            // counterpart. Otherwise, however, all data is copied to the clone-to form's 
            // "normal" list, which is what would be appropriate for duplicating a form and 
            // making an entirely new form.
            //
            std::vector<form_reference_t> locked;
            std::vector<form_reference_t> normal;
         } link_to; // TCLT[] // should be DIAL; xEdit claims it can be INFO too?
         struct {
            //
            // See note on (link_to).
            //
            components::condition_list locked;
            components::condition_list normal;
         } conditions; // CTDA
         std::vector<response> responses;
         localized_string override_topic_text; // RNAM (Prompt)
         components::papyrus_attachment_data script_data; // VMAD
         std::vector<components::legacy_script> legacy_scripts;

         // PNAM is not stored here; we handle it during the initial stub build

         constexpr float get_hours_until_reset() const noexcept {
            constexpr float conversion_divide = 65535.0F / 24.0F;
            return (float)this->raw_hours_until_reset / conversion_divide;
         }
         constexpr void set_hours_until_reset(float v) noexcept {
            if (v < 0)
               v = 0;
            else if (v > 65535.0F)
               v = 1;
            else
               v *= (65535.0F / 24.0F);
            this->raw_hours_until_reset = v;
         }

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}