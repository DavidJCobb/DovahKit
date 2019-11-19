#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "../components.h"
#include "../conditions.h"
#include "../papyrus.h"

class TESPluginRecord;

enum class quest_alias_type {
   reference,
   location,
};
SCOPE_ENUM(quest_alias_flags, enum quest_alias_flags : uint32_t {
   reserves_target      = 0x00000001,
   optional             = 0x00000002,
   quest_object         = 0x00000004, /* reference aliases only */
   allow_reuse_in_quest = 0x00000008,
   allow_dead           = 0x00000010, /* reference aliases only */
   limit_to_loaded_area = 0x00000020, /* reference aliases only - used for Find Matching Reference */
   make_essential       = 0x00000040, /* reference aliases only */
   allow_disabled       = 0x00000080, /* reference aliases only */
   stores_text          = 0x00000100,
   allow_reserved       = 0x00000200,
   make_protected       = 0x00000400, /* reference aliases only */
   no_fill_type         = 0x00000800, /* reference aliases only */
   allow_destroyed      = 0x00001000, /* reference aliases only */
   use_closest          = 0x00002000, /* reference aliases only - used for Find Matching Reference - only if In Loaded Area is set */
   uses_stored_text     = 0x00004000, /* reference aliases only */
   initially_disabled   = 0x00008000, /* reference aliases only */
   allow_cleared        = 0x00010000, /* location aliases only */
   clear_name_when_removed = 0x00020000, /* reference aliases only */
});
enum class location_alias_fill_type {
   none,
   preset, // ALFL: a preset Location form is "forced" into this alias
   other_alias_in_same_quest, // ALFA
   from_event, // ALFE
   other_alias_in_other_quest, // ALEQ
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

namespace LoadedForms {
   class Alias {
      public:
         virtual void load(TESPluginRecord&) = 0;
         //
         uint32_t    id = 0;
         std::string name;
         uint32_t    flags = 0; // enum is (quest_alias_flags)
         uint32_t    hiddenFlags = 0; // BNAM sets flag 0x01, ONAM sets flag 0x02
         uint32_t    forceIntoAliasID  = 0xFFFFFFFF; // same sentinel value used by the game
         uint32_t    fillFromEvent     = 0xFFFFFFFF; // same sentinel value used by the game
         uint32_t    fillFromEventData = 0xFFFFFFFF; // same sentinel value used by the game
         std::vector<Condition> conditions; // for "Find Matching Reference" or "Find Matching Location"
   };
   class LocationAlias : public Alias {
      public:
         typedef location_alias_fill_type fill_type;
         //
         virtual void load(TESPluginRecord&) override;
         //
         fill_type fillType = location_alias_fill_type::none;
         form_id_t fillFromLocationID = 0;
         form_id_t fillFromLocationKeywordID = 0;
         uint32_t  fillFromAliasID = 0xFFFFFFFF; // same sentinel value used by the game
         form_id_t fillFromQuestID = 0;
   };
   class ReferenceAlias : public Alias {
      public:
         typedef reference_alias_fill_type fill_type;
         struct InventoryModification {
            form_id_t itemFormID;
            uint32_t  count;
         };
      public:
         virtual void load(TESPluginRecord&) override;
         //
         fill_type fillType = reference_alias_fill_type::none;
         std::vector<form_id_t> keywordIDs; // KSIZ, KWDA
         std::vector<form_id_t> perkIDs;    // PRKZ, PRKR
         std::vector<form_id_t> packageIDs; // ALPC
         std::vector<form_id_t> factionIDs; // ALFC
         std::vector<form_id_t> spellIDs;   // ALSP
         std::vector<InventoryModification> inventoryChanges; // COCT, CNTO
         form_id_t spectatorOverridePackageListID = 0; // SPOR
         form_id_t observeCorpseOverridePackageListID = 0; // OCOR
         form_id_t guardWarnOverridePackageListID = 0; // GWOR
         form_id_t combatOverridePackageListID = 0; // ECOR
         form_id_t displayNameID = 0; // ALDN; should be the form ID of a MESG
         form_id_t additionalVoiceTypeID = 0; // VTCK; xEdit says can be the ID of a VTYP; UESP says can also be the ID of a FLST?
         //
         form_id_t fillLocRefTypeID = 0; // ALRT; should be the form ID of an LCRT
         uint32_t  fillNearAlias = 0xFFFFFFFF; // ALNA
         uint32_t  fillNearAliasType = 0; // ALNT
         form_id_t fillFromObjectReferenceID = 0;
         form_id_t createObjectBaseID = 0; // ALCO
         uint32_t  createObjectAt     = 0; // ALCA; sign bit is a flag; the rest is the alias ID
         uint32_t  createObjectLevel  = 0; // ALCL
         uint32_t  fillFromAliasID = 0xFFFFFFFF; // same sentinel value used by the game
         form_id_t fillFromQuestID = 0;
         form_id_t fillFromUniqueActorBaseID = 0; // ALUA; should be the form ID of an NPC_ with the Unique flag set
   };

   class Quest : public Form {
      public:
         Quest() : Form(FormType::Quest) {};
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
            std::vector<Condition> conditions;
            LStringRef journalText;
            uint32_t nextQuestID;
            // TODO: SCHR
            //
            void load(TESPluginRecord&); // assumes QSTD subrecord has already been opened
            void loadText(TESPluginSubrecord& cnam);
            void loadObScript(TESPluginRecord&);
         };
         struct Stage {
            uint16_t index;
            uint8_t  flags;
            uint8_t  padding;
            std::vector<LogEntry> entries;
            //
            void load(TESPluginSubrecord&); // assumes INDX subrecord has already been opened
         };

         struct Target { // QSTA
            uint32_t aliasID;
            uint8_t  flags; // stored in the file as a uint32_t, but loaded as a uint8_t; the game doesn't BSWAP if the endianness is wrong, so it must be a single byte with three padding bytes
            std::vector<Condition> conditions;
            //
            void load(TESPluginSubrecord&);
            void load(TESPluginRecord&); // assumes QSTA subrecord has already been opened
         };
         struct Objective {
            uint16_t   index;
            uint32_t   flags;
            LStringRef text;
            std::vector<Target> targets;
            bool error_isMissingFlags = false;
            bool error_isMissingText  = false;
            //
            void load(TESPluginSubrecord&);
            void load(TESPluginRecord&); // assumes QOBJ subrecord has already been opened
         };

         std::string editorID;
         LStringRef  name;
         PapyrusScriptData scriptData; // VMAD
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
         std::vector<Condition> dialogueConditions;
         std::vector<Condition> eventConditions;
         uint32_t event = 0;
         int32_t  nextAliasID = 0;
         std::vector<Stage> stages;
         std::vector<Objective> objectives;
         std::vector<Alias*> aliases;
         std::vector<form_id_t> textDisplayGlobalIDs;

         void load(TESPluginRecord&);

         static const char* QuestTypeToString(QuestType);

      protected:
         Target* getLastParsedQuestTarget() const noexcept;
   };
}