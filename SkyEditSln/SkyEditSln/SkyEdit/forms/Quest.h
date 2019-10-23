#pragma once
#include <cstdint>
#include <string>
#include "types.h"
#include "components.h"
#include "papyrus.h"

class TESPluginRecord;

class TESQuest : public TESForm {
   public:
      TESQuest() : TESForm(FormType::Quest) {};
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

      void load(TESPluginRecord&);

      static const char* QuestTypeToString(QuestType);
};