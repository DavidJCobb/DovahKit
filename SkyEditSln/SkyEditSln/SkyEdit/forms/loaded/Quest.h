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
enum class location_alias_fill_type {
   none,
   preset, // ALFL: a preset Location form is "forced" into this alias
   other_alias_in_same_quest, // ALFA
   from_event, // ALFE
   other_alias_in_other_quest, // ALEQ
};

namespace LoadedForms {
   class Alias {
      public:
         virtual void load(TESPluginRecord&) = 0;
         //
         uint32_t    id = 0;
         std::string name;
         uint32_t    flags = 0;
         uint32_t    hiddenFlags = 0; // BNAM sets flag 0x01, ONAM sets flag 0x02
         uint32_t    forceIntoAliasID  = 0xFFFFFFFF; // same sentinel value used by the game
         uint32_t    fillFromEvent     = 0xFFFFFFFF; // same sentinel value used by the game
         uint32_t    fillFromEventData = 0xFFFFFFFF; // same sentinel value used by the game
         std::vector<Condition> conditions;
   };
   class LocationAlias : public Alias {
      public:
         typedef location_alias_fill_type fill_type;
         //
         virtual void load(TESPluginRecord&) override;
         //
         fill_type fillType = location_alias_fill_type::none;
         uint32_t  fillFromLocationID = 0;
         uint32_t  fillFromLocationKeywordID = 0;
         uint32_t  fillFromAliasID = 0xFFFFFFFF; // same sentinel value used by the game
         uint32_t  fillFromQuestID = 0;
   };
   class ReferenceAlias : public Alias {
      public:
         virtual void load(TESPluginRecord&) override;
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
         std::vector<uint32_t> textDisplayGlobalIDs;

         void load(TESPluginRecord&);

         static const char* QuestTypeToString(QuestType);
   };
}