#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/conditions.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class TopicInfo : public Form {
      //
      // TODO: TopicInfo coalesces data from the last (series of) partial record(s) just before 
      // the winning record. This means that if an info is overridden in the active file and 
      // the records just before that override are partial-flagged, we'll need to be extra 
      // careful how we save it: if we save the loaded data verbatim (as we usually would), 
      // then all non-response data will be duplicated at run-time. This is fine for single 
      // fields since that's functionally the same as overriding them, but lists -- including 
      // the "link to" list and the condition list -- will have their contents duplicated.
      //
      // Unfortunately, that means that we'll need to split all lists into "winning" and 
      // "merged" sub-lists.
      //
      // This doesn't apply to responses because again, those are pulled only from the winning 
      // record.
      //
      // Some additional considerations:
      //
      //  - Modifying the "merged" lists should be considered undefined behavior. Functions 
      //    like reference-severing and clearing should still process them, but if those 
      //    functions are ever invoked in a way that actually touches them then that's UB 
      //    as well. (We should never sever references to a form outside of the active file 
      //    because we cannot delete those.)
      //
      //     - Remember: we can't even modify these, let alone delete them, because all we 
      //       can do if we've overridden a partial INFO is add to it. Any merged items need 
      //       to be greyed out in the UI, or otherwise have it communicated that they can't 
      //       be edited, deleted, reordered, or otherwise altered in any way.
      //
      //       This case isn't going to come up often (and perhaps shouldn't come up ever), 
      //       so don't even bother trying to make it "intuitive." We have enough to deal 
      //       with already.
      //
      //  - When cloning a TopicInfo, the clone's "winning" list should contain the contents 
      //    of the source's "merged" and "winning" lists, in that order.
      //
      public:
         static constexpr form_type_t form_type = form_type::topic_info;
         TopicInfo(const constructor_params& c) : Form(form_type, c) {};

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
            //
            struct {
               int32_t type  =  0;
               int32_t value = 50;
            } emotion;
            uint32_t unused;
            uint32_t response_number;
            // 3 padding bytes here
            form_reference_t sound;
            flags_t flags = 0;
            localized_string text;
            localized_string script_notes;
            localized_string edits;
            struct {
               form_reference_t speaker;
               form_reference_t listener;
            } idles;
         };

         info_flags_t  info_flags  = 0;
         load_flags_t  load_flags  = 0;
         favor_level_t favor_level = favor_level_t::none; // CNAM
         float         days_until_reset = 0.0F;
         form_reference_t speaker; // ANAM
         form_reference_t topic; // TPIC // unknown
         form_reference_t walk_away_topic; // TWAT (yes, really)
         form_reference_t use_shared_info; // DNAM // a SharedInfo to borrow response data from
         form_reference_t audio_override_output; // ONAM
         std::vector<form_reference_t> link_to; // TCLT[] // should be DIAL; xEdit claims it can be INFO too?
         std::vector<components::condition> conditions; // CTDA
         std::vector<response> responses;
         localized_string override_topic_text; // RNAM (Prompt)
         components::object_bounds object_bounds; // OBND. recognized, but probably discarded at run-time.
         components::papyrus_attachment_data script_data; // VMAD

         // PNAM is not stored here; we handle it during the initial stub build

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc); // TODO: FINISH ME
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
   };
}