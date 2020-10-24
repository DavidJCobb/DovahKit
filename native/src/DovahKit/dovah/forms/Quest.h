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

enum class quest_alias_type {
   reference,
   location,
};
enum class reference_alias_fill_type {
   none                       = 0,
   preset_placed_reference    = 1, // ALFR: a preset Actor or ObjectReference is "forced" into this alias
   other_alias_in_same_quest  = 2, // ALFA
   from_event                 = 3, // ALFE
   create_object              = 4, // ALCO
   other_alias_in_other_quest = 5, // ALEQ
   preset_unique_actor        = 6, // ALUA
   find_matching_reference    = 7, // ALNA
};

namespace dovah::loaded_forms {
   class Alias {
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
         virtual void load(tes_record_reader&) = 0;
         //
         uint32_t    id = 0;
         std::string name;
         flags_t     flags = 0;
         uint32_t    hiddenFlags = 0; // BNAM sets flag 0x01, ONAM sets flag 0x02
         uint32_t    forceIntoAliasID  = 0xFFFFFFFF; // same sentinel value used by the game
         uint32_t    fill_from_event      = 0xFFFFFFFF; // same sentinel value used by the game
         uint32_t    fill_from_event_data = 0xFFFFFFFF; // same sentinel value used by the game
         std::vector<components::condition> conditions; // for "Find Matching Reference" or "Find Matching Location"
   };
   class LocationAlias : public Alias {
      public:
         enum class fill_type_t {
            none,
            preset, // ALFL: a preset Location form is "forced" into this alias
            other_alias_in_same_quest, // ALFA
            from_event, // ALFE
            other_alias_in_other_quest, // ALEQ
         };
         //
         virtual void load(tes_record_reader&) override;
         //
         fill_type_t      fill_type = fill_type_t::none;
         form_reference_t fill_from_location;
         form_reference_t fill_from_location_keyword;
         uint32_t         fill_from_alias = 0xFFFFFFFF; // same sentinel value used by the game
         form_reference_t fill_from_quest;
   };
   class ReferenceAlias : public Alias {
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
      public:
         virtual void load(tes_record_reader&) override;
         //
         fill_type_t fillType = fill_type_t::none;
         components::keyword_list   keywords; // KSIZ, KWDA
         components::container_data inventory;
         std::vector<form_reference_t> perks;    // PRKZ, PRKR
         std::vector<form_reference_t> packages; // ALPC
         std::vector<form_reference_t> factions; // ALFC
         std::vector<form_reference_t> spells;   // ALSP
         form_reference_t spectatorOverridePackageListID; // SPOR
         form_reference_t observeCorpseOverridePackageListID; // OCOR
         form_reference_t guardWarnOverridePackageListID; // GWOR
         form_reference_t combatOverridePackageListID; // ECOR
         form_reference_t displayNameID; // ALDN; should be the form ID of a MESG
         form_reference_t additionalVoiceTypeID; // VTCK; xEdit says can be the ID of a VTYP; UESP says can also be the ID of a FLST?
         //
         form_reference_t fillLocRefTypeID; // ALRT; should be the form ID of an LCRT
         uint32_t  fillNearAlias = 0xFFFFFFFF; // ALNA
         uint32_t  fillNearAliasType = 0; // ALNT
         form_reference_t fillFromObjectReferenceID;
         form_reference_t createObjectBaseID; // ALCO
         uint32_t  createObjectAt     = 0; // ALCA; sign bit is a flag; the rest is the alias ID
         uint32_t  createObjectLevel  = 0; // ALCL
         uint32_t  fillFromAliasID = 0xFFFFFFFF; // same sentinel value used by the game
         form_reference_t fillFromQuestID;
         form_reference_t fillFromUniqueActorBaseID; // ALUA; should be the form ID of an NPC_ with the Unique flag set
   };

   class Quest : public Form {
      public:
         static constexpr form_type_t form_type = form_type::quest;
         Quest() : Form(form_type) {};
         ~Quest();
         //
         enum QuestFlags : uint16_t {
            kQuestFlag_StartGameEnabled = 1,
            kQuestFlag_AllowRepeatedStages = 8,
            kQuestFlag_RunOnce = 0x100,
            kQuestFlag_ExcludeFromDialogueExport = 0x0200,
            kQuestFlag_WarnOnAliasFillFailure = 0x0400,
         };
         enum QuestType : uint8_t {
            kQuestType_None = 0,
            kQuestType_Main = 1,
            kQuestType_MagesGuild = 2,
            kQuestType_ThievesGuild = 3,
            kQuestType_DarkBrotherhood = 4,
            kQuestType_Companions = 5,
            kQuestType_Miscellaneous = 6,
            kQuestType_Daedric = 7,
            kQuestType_Sidequest = 8,
            kQuestType_CivilWar = 9,
            kQuestType_DLC01Vampire = 10,
            kQuestType_DLC02Dragonborn = 11,
         };

         struct LogEntry {
            uint8_t flags;
            std::vector<components::condition> conditions;
            localized_string journalText = localized_string(localized_string_type::description);
            form_reference_t nextQuestID;
            // TODO: SCHR
            //
            void load(tes_record_reader&); // assumes QSTD subrecord has already been opened
            void loadText(tes_subrecord_reader& cnam);
            void loadObScript(tes_record_reader&);
         };
         struct Stage {
            uint16_t index;
            uint8_t  flags;
            uint8_t  padding;
            std::vector<LogEntry> entries;
            //
            void load(tes_subrecord_reader&); // assumes INDX subrecord has already been opened
         };

         struct Target { // QSTA
            uint32_t aliasID;
            uint8_t  flags; // stored in the file as a uint32_t, but loaded as a uint8_t; the game doesn't BSWAP if the endianness is wrong, so it must be a single byte with three padding bytes
            std::vector<components::condition> conditions;
            //
            void load(tes_subrecord_reader&); // assumes QSTA subrecord has already been opened
         };
         struct Objective {
            uint16_t   index;
            uint32_t   flags;
            localized_string text;
            std::vector<Target> targets;
            bool error_isMissingFlags = false;
            bool error_isMissingText  = false;
            //
            void load(tes_subrecord_reader&);
            void load(tes_record_reader&); // assumes QOBJ subrecord has already been opened
         };

         localized_string  name;
         components::papyrus_attachment_data scriptData; // VMAD
         //
         // DNAM:
         //
         uint16_t    flags = 0;
         uint8_t     priority;
         uint8_t     formVersion = 0;
         uint32_t    unknown; // DNAM, offset 0x04
         QuestType   questType;
         //
         std::string editorCategory; // FLTR // "abc/def/ghi" to nest within the CK Object Window tree
         std::vector<components::condition> dialogueConditions;
         std::vector<components::condition> eventConditions;
         uint32_t event = 0;
         int32_t  nextAliasID = 0;
         std::vector<Stage> stages;
         std::vector<Objective> objectives;
         std::vector<Alias*> aliases;
         std::vector<form_reference_t> textDisplayGlobalIDs;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

         static const char* QuestTypeToString(QuestType);

      protected:
         Target* getLastParsedQuestTarget() const noexcept;
   };
}