#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/container.h"
#include "components/keyword_list.h"
#include "components/papyrus.h"
#include "../data/story_manager.h"

namespace dovah::loaded_forms {
   using alias_id_t = uint32_t;

   class Quest;

   class Alias {
      friend class Quest;
      public:
         struct flag {
            flag() = delete;
            enum type : uint32_t {
               reserves_target         = 0x00000001,
               optional                = 0x00000002,
               quest_object            = 0x00000004, // reference aliases only
               allow_reuse_in_quest    = 0x00000008,
               allow_dead              = 0x00000010, // reference aliases only
               limit_to_loaded_area    = 0x00000020, // reference aliases only; used for Find Matching Reference
               make_essential          = 0x00000040, // reference aliases only
               allow_disabled          = 0x00000080, // reference aliases only
               stores_text             = 0x00000100,
               allow_reserved          = 0x00000200,
               make_protected          = 0x00000400, // reference aliases only
               no_fill_type            = 0x00000800, // reference aliases only
               allow_destroyed         = 0x00001000, // reference aliases only
               use_closest             = 0x00002000, // reference aliases only; used for Find Matching Reference; only if In Loaded Area is set
               uses_stored_text        = 0x00004000, // reference aliases only
               initially_disabled      = 0x00008000, // reference aliases only
               allow_cleared           = 0x00010000, // location aliases only
               clear_name_when_removed = 0x00020000, // reference aliases only
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;
         //
         enum class alias_type {
            undifferentiated,
            reference,
            location,
         };
         //
         const alias_type type = alias_type::undifferentiated;
         uint32_t    id = 0;
         std::string name;
         flags_t     flags = 0;
         uint32_t    hidden_flags = 0; // BNAM sets flag 0x01, ONAM sets flag 0x02
         alias_id_t  force_into_alias_id  = 0xFFFFFFFF; // same sentinel value used by the game
         story_event_code_t fill_from_event      = story_event_code::undefined; // same sentinel value used by the game
         uint32_t           fill_from_event_data; // e.g. 4C 32 00 00 -> 'L2'
         std::vector<components::condition> conditions; // for "Find Matching Reference" or "Find Matching Location"
         //
         Alias(alias_type at) : type(at) {}
         //
      protected:
         virtual void load(tes_record_reader&, load_order_interfaces::form_load&) = 0;
         virtual void save(tes_record_writer&, load_order_interfaces::form_save&) = 0;
         virtual void sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept = 0;
         virtual Alias* clone(loaded_forms::Form& clone_owner) = 0;
         virtual void clear(loaded_forms::Form& my_owner) = 0;
   };
   class LocationAlias : public Alias {
      friend class Quest;
      public:
         enum class fill_type_t {
            none,
            preset, // ALFL: a preset Location form is "forced" into this alias
            other_alias_in_same_quest, // ALFA
            from_event, // ALFE
            other_alias_in_other_quest, // ALEQ
         };
         //
         fill_type_t      fill_type = fill_type_t::none;
         form_reference_t fill_from_location;
         form_reference_t fill_from_location_keyword;
         alias_id_t       fill_from_alias = 0xFFFFFFFF; // ALEQ:ALEA or ALFA // same sentinel value used by the game
         form_reference_t fill_from_quest; // ALEQ
         //
         LocationAlias() : Alias(alias_type::location) {}
         //
      protected:
         virtual void load(tes_record_reader&, load_order_interfaces::form_load&) override;
         virtual void save(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual void sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept override;
         virtual Alias* clone(loaded_forms::Form& clone_owner) override;
         virtual void clear(loaded_forms::Form& my_owner) override;
   };
   class ReferenceAlias : public Alias {
      friend class Quest;
      public:
         enum class fill_type_t {
            none                       = 0,
            preset_placed_reference    = 1, // ALFR: a preset Actor or ObjectReference is "forced" into this alias
            other_alias_in_same_quest  = 2, // ALFA
            from_event                 = 3, // ALFE
            create_object              = 4, // ALCO
            other_alias_in_other_quest = 5, // ALEQ
            preset_unique_actor        = 6, // ALUA
            find_matching_reference    = 7, // ALNA
         };
         //
         fill_type_t fill_type = fill_type_t::none;
         components::keyword_list      keywords; // KSIZ, KWDA
         components::container_data    inventory;
         std::vector<form_reference_t> packages; // ALPC
         std::vector<form_reference_t> factions; // ALFC
         std::vector<form_reference_t> spells;   // ALSP
         struct {
            form_reference_t spectator; // SPOR
            form_reference_t observe_corpse; // OCOR
            form_reference_t guard_warn; // GWOR
            form_reference_t combat; // ECOR
         } package_override_lists;
         form_reference_t display_name; // ALDN; should be the form ID of a MESG
         form_reference_t additional_voicetype; // VTCK; xEdit says can be the ID of a VTYP; UESP says can also be the ID of a FLST?
         //
         form_reference_t fill_loc_ref_type; // ALRT; should be the form ID of an LCRT
         alias_id_t       fill_near_alias = 0xFFFFFFFF; // ALNA
         uint32_t         fill_near_alias_type = 0; // ALNT
         form_reference_t fill_from_reference;
         form_reference_t create_object_of_type; // ALCO
         alias_id_t       create_object_at_alias = 0; // ALCA; sign bit is a flag (create inside of / create at); the rest is the alias ID
         uint32_t         create_object_of_level = 0; // ALCL
         alias_id_t       fill_from_alias = 0xFFFFFFFF; // same sentinel value used by the game
         form_reference_t fill_from_quest;
         form_reference_t fill_from_unique_actor_base; // ALUA; should be the form ID of an NPC_ with the Unique flag set
         //
         ReferenceAlias() : Alias(alias_type::reference) {}
         //
      protected:
         virtual void load(tes_record_reader&, load_order_interfaces::form_load&) override;
         virtual void save(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual void sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept override;
         virtual Alias* clone(loaded_forms::Form& clone_owner) override;
         virtual void clear(loaded_forms::Form& my_owner) override;
   };

   class Quest : public Form {
      public:
         class Stage;
         class Objective;
      public:
         static constexpr form_type_t form_type = form_type::quest;
         Quest() : Form(form_type) {};
         ~Quest();
         
         struct quest_flag {
            enum type : uint16_t {
               start_game_enabled           = 0x0001,
               allow_repeated_stages        = 0x0008,
               run_once                     = 0x0100,
               exclude_from_dialogue_export = 0x0200,
               warn_on_alias_fill_failure   = 0x0400,
            };
         };
         using quest_flags_t = std::underlying_type_t<quest_flag::type>;
         
         struct quest_type {
            enum type : uint8_t {
               none             =  0,
               main             =  1,
               mages_guild      =  2,
               thieves_guild    =  3,
               dark_brotherhood =  4,
               companions       =  5,
               miscellaneous    =  6,
               daedric          =  7,
               sidequest        =  8,
               civil_war        =  9,
               dlc_dawnguard    = 10,
               dlc_dragonborn   = 11,
            };
         };
         using quest_type_t = std::underlying_type_t<quest_type::type>;

         class LogEntry {
            friend class Quest;
            friend class Quest::Stage;
            public:
               struct flag {
                  enum type : uint8_t {
                     complete = 0x01,
                     fail     = 0x02,
                  };
               };
               using flags_t = std::underlying_type_t<flag::type>;
               //
               flags_t flags;
               std::vector<components::condition> conditions;
               localized_string journal_text = localized_string(localized_string_type::description);
               form_reference_t next_quest_id;
               //
            protected:
               void load(tes_record_reader&,    load_order_interfaces::form_load&); // assumes QSTD subrecord has already been opened
               void load(tes_subrecord_reader&, load_order_interfaces::form_load&);
               bool save(tes_record_writer&,    load_order_interfaces::form_save&);
               void sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept;
               void clone_from(const LogEntry&, loaded_forms::Form&);
               void clear(loaded_forms::Form&);
         };
         class Stage {
            friend class Quest;
            public:
               struct flag {
                  enum type : uint8_t {
                     startup  = 0x02,
                     shutdown = 0x04,
                     keep_instance_data = 0x08,
                  };
               };
               using flags_t = std::underlying_type_t<flag::type>;
               //
               uint16_t index;
               flags_t  flags;
               uint8_t  padding;
               std::vector<LogEntry> entries;
               //
            protected:
               void load(tes_subrecord_reader&, load_order_interfaces::form_load&); // assumes INDX subrecord has already been opened
               bool save(tes_record_writer&,    load_order_interfaces::form_save&);
               void sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept;
               void clone_from(const Stage&, loaded_forms::Form&);
               void clear(loaded_forms::Form&);
         };

         class Target { // QSTA
            friend class Quest;
            friend class Quest::Objective;
            public:
               struct flag {
                  enum type : uint8_t {
                     marker_pathing_ignores_locks = 0x00000001,
                  };
               };
               using flags_t = std::underlying_type_t<flag::type>;
               //
               uint32_t aliasID = -1;
               flags_t  flags   =  0; // stored in the file as a uint32_t, but loaded by the game as a uint8_t; the game doesn't BSWAP if the endianness is wrong, so it must be a single byte with three padding bytes
               std::vector<components::condition> conditions;
               //
            protected:
               void load(tes_subrecord_reader&, load_order_interfaces::form_load&); // assumes QSTA subrecord has already been opened
               bool save(tes_record_writer&, load_order_interfaces::form_save&);
               void sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept;
               void clone_from(const Target&, loaded_forms::Form&);
               void clear(loaded_forms::Form&);
         };
         class Objective {
            friend class Quest;
            public:
               struct flag {
                  enum type : uint32_t {
                     or_with_previous = 0x00000001,
                  };
               };
               using flags_t = std::underlying_type_t<flag::type>;
               //
               uint16_t index;
               flags_t  flags = 0;
               localized_string    text;
               std::vector<Target> targets;
               //
            protected:
               void load(tes_record_reader&, load_order_interfaces::form_load&); // assumes QOBJ subrecord has already been opened
               bool save(tes_record_writer&, load_order_interfaces::form_save&);
               void sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept;
               void clone_from(const Objective&, loaded_forms::Form&);
               void clear(loaded_forms::Form&);
         };

         localized_string name;
         components::papyrus_attachment_data script_data; // VMAD
         //
         // DNAM:
         //
         quest_flags_t flags = 0;
         uint8_t       priority;
         uint8_t       form_version = 0;
         uint32_t      unknown; // DNAM, offset 0x04
         quest_type_t  quest_type;
         //
         std::string editor_category; // FLTR // "abc/def/ghi" to nest within the CK Object Window tree
         struct {
            std::vector<components::condition> dialogue;
            std::vector<components::condition> event;
         } conditions;
         story_event_code_t event         = story_event_code::none;
         int32_t            next_alias_id = 0;
         std::vector<Stage> stages;
         std::vector<Objective> objectives;
         std::vector<Alias*> aliases;
         std::vector<form_reference_t> text_display_globals;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
         virtual components::papyrus_attachment_data* get_papyrus_data() noexcept override { return &this->script_data; }

         Alias* lookup_alias_by_id(uint32_t id) const noexcept;

      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}