#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../types.h"
#include "../../helpers/scoped_enum.h"
#include "model.h"

class FormStub;
class TESPluginSubrecord;

SCOPE_ENUM(destruction_data_flags, enum destruction_data_flags : uint8_t {
   vats_enabled = 0x01,
});
SCOPE_ENUM(destruction_stage_flags, enum destruction_stage_flags : uint8_t {
   cap_damage     = 0x01, SCOPED_ENUM_COMMENT("When the object is at this stage, prevents you from skipping multiple stages by inflicting massive damage.")
   disable_object = 0x02,
   destroy_object = 0x04,
   ignore_external_damage = 0x08, SCOPED_ENUM_COMMENT("Prevents the object from taking damage from anything other than Self Damage Per Second.")
});
struct DestructionStageData {
   struct Stage {
      uint16_t  healthPercent;
      uint8_t   damageStage;
      uint8_t   flags;
      uint32_t  selfDamageRate; // the object inflicts this much damage on itself per second (i.e. health decay)
      form_id_t explosionID;
      form_id_t debrisID;
      uint32_t  debrisCount;
      FormModel replacementModel;
   };
   //
   uint32_t health;
   uint8_t  flags;
   std::vector<Stage> stages;
   //
   void load(TESPluginSubrecord&);
   static void generateUseInfo(TESPluginSubrecord&, FormStub*);
};