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

namespace LoadedForms {
   class Quest : public Form {
      public:
         Quest() : Form(FormType::Quest) {};
         //
         enum QuestFlags : uint16_t {
            kQuestFlag_StartGameEnabled = 1,
            kQuestFlag_AllowRepeatedStages = 8,
            kQuestFlag_RunOnce = 0x100,
            kQuestFlag_ExcludeFromDialogueExport = 0x0200,
            kQuestFlag_WarnOnAliasFillFailure = 0x0400,
         };
         enum QuestType : uint32_t {
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
            void load(TESPluginRecord&);
         };
         struct Stage {
            uint16_t index;
            uint8_t  flags;
            uint8_t  padding;
            std::vector<LogEntry> entries;
            //
            void load(TESPluginRecord&);
         };

         struct Target {
            uint32_t aliasID;
            uint32_t flags;
            std::vector<Condition> conditions;
            //
            void load(TESPluginRecord&); // assumes QSTA subrecord has already been opened
         };
         struct Objective {
            uint16_t   index;
            uint32_t   flags;
            LStringRef text;
            std::vector<Target> targets;
            //
            void load(TESPluginRecord&); // assumes QOBJ subrecord has already been opened
         };

         struct Alias {
            quest_alias_type type;
            uint32_t    id;
            std::string name;
            uint32_t    flags;
            bool        hasForceInto = false;
            uint32_t    forceInto = 0;
            //
            // TODO: FINISH ME
            //
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
         uint32_t    unknown;
         QuestType   questType;
         //
         std::string editorCategory; // FLTR // "abc/def/ghi" to nest within the CK Object Window tree
         std::vector<Condition> dialogueConditions;
         std::vector<Condition> eventConditions;
         uint32_t event = 0;
         int32_t  nextAliasID = 0;
         std::vector<Stage> stages;
         std::vector<Objective> objectives;
         std::vector<Alias> aliases;

         void load(TESPluginRecord&);

         static const char* QuestTypeToString(QuestType);
   };
}