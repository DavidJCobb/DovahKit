#pragma once
#include <cstdint>
#include <string>
#include "types.h"
#include "components.h"

class TESPluginFile;

class TESQuest : public TESForm {
   public:
      TESQuest() : TESForm(77) {};
      //
      enum QuestFlags {
         kQuestFlag_StartGameEnabled = 1,
         kQuestFlag_AllowRepeatedStages = 8,
         kQuestFlag_RunOnce = 0x100,
         kQuestFlag_ExcludeFromDialogueExport = 0x0200,
         kQuestFlag_WarnOnAliasFillFailure    = 0x0400,
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

      std::string editorID;
      LStringRef  name;
      uint16_t    flags;
      uint8_t     priority;
      uint8_t     formVersion;
      uint32_t    unknown;
      QuestType   questType;

      void load(TESPluginFile*);

      static const char* QuestTypeToString(QuestType);
};