#pragma once
#include <cstdint>
#include <functional>
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
         
         enum class alias_type {
            undifferentiated,
            reference,
            location,
         };
         
         enum class fill_type_t {
            none = 0,
            //
            other_alias_in_same_quest,  // ALFA (typically followed by a type-specific field)
            from_event,                 // ALFE + ALFD
            other_alias_in_other_quest, // ALEQ
            //
            // Types unique to location aliases:
            //
            preset_location, // ALFL
            //
            // Types unique to reference aliases:
            //
            preset_placed_reference,    // ALFR: a preset Actor or ObjectReference is "forced" into this alias
            create_object,              // ALCO
            preset_unique_actor,        // ALUA
            find_matching_reference,    // ALNA
         };

         static constexpr alias_id_t none_id = 0xFFFFFFFF;

         Alias(Quest& owner, alias_type at) : owner(owner), type(at) {}
         
         Quest& owner;
         const alias_type type = alias_type::undifferentiated;
         uint32_t    id = 0;
         std::string name;
         flags_t     flags = 0;
         uint32_t    hidden_flags = 0; // BNAM sets flag 0x01, ONAM sets flag 0x02
         alias_id_t  force_into_alias_id  = none_id; // same sentinel value used by the game
         //
         fill_type_t fill_type = fill_type_t::none;
         struct {
            form_reference_t quest;           // ALEQ // if nullptr, then the specified alias is inside of this alias's containing quest
            alias_id_t       alias = none_id; // ALEA, ALFA
         } fill_from_alias;
         struct {
            story_event_code_t code   = story_event_code::undefined; // ALFE // same sentinel value used by the game
            uint32_t           member = 0; // ALFD
         } fill_from_event;
         //
         components::condition_list       conditions; // for "Find Matching Reference" or "Find Matching Location"
         components::papyrus::script_data script_data;

         void load(tes_record_reader&, load_order_interfaces::form_load&);
         void save(tes_record_writer&, load_order_interfaces::form_save&);
         void sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept;
         Alias* clone(loaded_forms::Form& clone_owner);
         void clear(loaded_forms::Form& my_owner);
         //
         static alias_id_t generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual bool _load_impl(tes_subrecord_reader&, load_order_interfaces::form_load&) = 0;
         virtual bool _save_fill_impl(tes_record_writer&, load_order_interfaces::form_save&) = 0; // handle the fill-type, if it is (or has any data) specific to the alias type. return true if type handled; false if not
         virtual void _save_body_impl(tes_record_writer&, load_order_interfaces::form_save&) = 0;
         virtual void _sever_outbound_references_impl(form_stub& target, loaded_forms::Form& my_owner) noexcept = 0;
         virtual Alias* _clone_impl(loaded_forms::Form& clone_owner) = 0;
         virtual void _clear_impl(loaded_forms::Form& my_owner) = 0;
   };
   class LocationAlias : public Alias {
      friend class Alias;
      friend class Quest;
      public:
         form_reference_t fill_from_location;
         form_reference_t fill_from_location_keyword;
         //
         LocationAlias(Quest& o) : Alias(o, alias_type::location) {}
         //
      protected:
         virtual bool _load_impl(tes_subrecord_reader&, load_order_interfaces::form_load&) override;
         virtual bool _save_fill_impl(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual void _save_body_impl(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual void _sever_outbound_references_impl(form_stub& target, loaded_forms::Form& my_owner) noexcept override;
         virtual Alias* _clone_impl(loaded_forms::Form& clone_owner) override;
         virtual void _clear_impl(loaded_forms::Form& my_owner) override;
         //
         struct _use_info_field_state {
            form_id_t fill_from_location;
            form_id_t fill_from_location_keyword;
         };
         static void generate_use_info_for_subrecord(_use_info_field_state&, tes_subrecord_reader&, form_stub_use_info_builder&);
   };
   class ReferenceAlias : public Alias {
      friend class Alias;
      friend class Quest;
      public:
         components::keyword_list      keywords; // KSIZ, KWDA
         components::container_data    inventory;
         std::vector<form_reference_t> packages; // ALPC
         std::vector<form_reference_t> factions; // ALFC
         std::vector<form_reference_t> spells;   // ALSP
         struct {
            form_reference_t spectator;      // SPOR
            form_reference_t observe_corpse; // OCOR
            form_reference_t guard_warn;     // GWOR
            form_reference_t combat;         // ECOR
         } package_override_lists; // same structure as on NPC_
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
         form_reference_t fill_from_unique_actor_base; // ALUA; should be the form ID of an NPC_ with the Unique flag set
         //
         ReferenceAlias(Quest& o) : Alias(o, alias_type::reference) {}
         //
      protected:
         virtual bool _load_impl(tes_subrecord_reader&, load_order_interfaces::form_load&) override;
         virtual bool _save_fill_impl(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual void _save_body_impl(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual void _sever_outbound_references_impl(form_stub& target, loaded_forms::Form& my_owner) noexcept override;
         virtual Alias* _clone_impl(loaded_forms::Form& clone_owner) override;
         virtual void _clear_impl(loaded_forms::Form& my_owner) override;
         //
         struct _use_info_field_state {
            struct {
               form_id_t spectator; // SPOR
               form_id_t observe_corpse; // OCOR
               form_id_t guard_warn; // GWOR
               form_id_t combat; // ECOR
            } package_override_lists;
            form_id_t display_name;
            form_id_t additional_voicetype;
            form_id_t fill_loc_ref_type;
            form_id_t fill_from_reference;
            form_id_t create_object_of_type;
            form_id_t fill_from_unique_actor_base;
         };
         static void generate_use_info_for_subrecord(_use_info_field_state&, tes_subrecord_reader&, form_stub_use_info_builder&);
   };

   class Quest : public Form {
      #include "impl/form_subclass_components.txt"
      public:
         class Stage;
         class Objective;
      public:
         static constexpr form_type_t form_type = form_type::quest;
         Quest(const constructor_params& c) : Form(form_type, c) {};
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

               struct script_fragment {
                  uint16_t    stage_id; // not meaningful outside of load
                  uint16_t    unknown02 = 0x0000;
                  uint32_t    entry_index; // not meaningful outside of load
                  uint8_t     unknown08 = 0x01;
                  std::string filename; // read as a length-prefixed string, capped to 65535 chars
                  std::string function; // read as a length-prefixed string, capped to 65535 chars
                  //
                  void load(tes_subrecord_reader&); // read from VMAD
                  bool save(tes_subrecord_writer&, uint16_t stage_id, uint32_t entry_index); // write to VMAD
                  static void generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib);
                  void clear();
                  //
                  inline bool empty() const noexcept {
                     return this->filename.empty() || this->function.empty();
                  }
               };
               
               flags_t flags = 0;
               localized_string journal_text = localized_string(localized_string_type::description);
               form_reference_t next_quest_id;
               components::condition_list conditions;
               script_fragment fragment;
               
            protected:
               void load(tes_record_reader&,    load_order_interfaces::form_load&); // assumes QSTD subrecord has already been opened
               void load(tes_subrecord_reader&, load_order_interfaces::form_load&);
               bool save(tes_record_writer&,    load_order_interfaces::form_save&);
               void sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept;
               void clone_from(const LogEntry&, loaded_forms::Form& my_owner);
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
               std::vector<LogEntry> entries; // do not remove elements from here without first calling LogEntry::clear, or you risk breaking the quest's use info
               //
            protected:
               void load(tes_subrecord_reader&, load_order_interfaces::form_load&); // assumes INDX subrecord has already been opened
               bool save(tes_record_writer&,    load_order_interfaces::form_save&);
               void sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept;
               void clone_from(const Stage&, loaded_forms::Form& my_owner);
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
               components::condition_list conditions;
               //
            protected:
               void load(tes_subrecord_reader&, load_order_interfaces::form_load&); // assumes QSTA subrecord has already been opened
               bool save(tes_record_writer&, load_order_interfaces::form_save&);
               void sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept;
               void clone_from(const Target&, loaded_forms::Form& my_owner);
               void clear(loaded_forms::Form&);
         };
         class Objective {
            friend class Quest;
            public:
               struct flag {
                  enum type : uint32_t {
                     or_with_previous = 0x00000001, // prepends "or" to the objective text, in addition to any non-cosmetic behaviors it may have
                  };
               };
               using flags_t = std::underlying_type_t<flag::type>;
               //
               uint16_t index;
               flags_t  flags = 0;
               localized_string    text;
               std::vector<Target> targets; // do not remove elements from here without first calling Target::clear, or you risk breaking the quest's use info
               //
            protected:
               void load(tes_record_reader&, load_order_interfaces::form_load&); // assumes QOBJ subrecord has already been opened
               bool save(tes_record_writer&, load_order_interfaces::form_save&);
               void sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept;
               void clone_from(const Objective&, loaded_forms::Form& my_owner);
               void clear(loaded_forms::Form&);
         };

         localized_string name;
         components::papyrus_attachment_data script_data; // VMAD
         struct {
            uint8_t     unknown = 2; // if this is present in the file at all and isn't 2, then quest aliases don't load? what the hell?!
            std::string filename;
         } script_fragment_root;
         //
         // DNAM:
         //
         quest_flags_t flags      = 0;
         uint8_t       priority   = 50;
         float         unknown    = 1.0F; // DNAM, offset 0x04 // suspected to be leftover script interval from ObScript
         quest_type_t  quest_type = quest_type::none;
         //
         struct {
            components::condition_list dialogue;
            components::condition_list event;
         } conditions;
         story_event_code_t event         = story_event_code::none;
         int32_t            next_alias_id = 0;
         std::vector<Stage> stages;
         std::vector<Objective> objectives;
         std::vector<Alias*> aliases;
         std::vector<form_reference_t> text_display_globals;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

         Alias* lookup_alias_by_id(uint32_t id) const noexcept;
         void for_each_alias_of_type(Alias::alias_type, std::function<bool(Alias*)>);

         Stage* lookup_stage_by_id(uint16_t id) noexcept;
         Stage* insert_stage(int id) noexcept; // returns nullptr if a stage with that ID already exists or if the ID is out of bounds
         void   remove_stage(int id) noexcept;

         void remove_log_entry(int stage_id, int entry_index) noexcept; // needed to fix up use info

      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}