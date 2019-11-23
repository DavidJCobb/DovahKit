#pragma once
#include <cstdint>
#include <string>
#include "../helpers/scoped_enum.h"

class TESPluginRecord;

enum class ConditionParamType : uint8_t {
   None = 0,
   Actor,       // forms of type: ACHR
   ActorBase,   // forms of type: NPC_
   ActorValue,
   AdvanceAction,
   Alias,
   Alignment,   // enum; karma
   AssociationType, // forms of type: ASTP
   Axis,        // char
   BaseForm,
   BodyPart,    // integer: body part enum value
   CastingSource,
   Cell,        // forms of type: CELL
   Class,       // forms of type: CLAS
   CrimeType,
   CriticalStage, // enum
   EncounterZone, // forms of type: ECZN
   EquipType,
   Event,
   EventData,
   Faction,     // forms of type: FACT
   Float,
   FormList,    // forms of type: FLST
   FormType,
   Furniture,   // forms of type: FURN
   FurnitureAnimType,
   FurnitureEntryType,
   Global,      // forms of type: GLOB
   Idle,        // form
   Integer,
   InventoryItem, // forms of type: [todo; anything that can ever go in an inventory]
   Keyword,     // forms of type: KYWD
   KnowableForm,
   Location,    // forms of type: LCTN
   MagicEffect, // forms of type: MGEF
   MiscStat,
   ObjectReference, // forms of type: ACHR, REFR, or in theory any placed projectile or hazard
   Owner,       // forms of type: FACT, NPC_
   Package,     // forms of type: PACK
   PackageData,
   Perk,        // forms of type: PERK
   Quest,       // forms of type: QUST
   QuestStage,
   Race,        // forms of type: RACE
   RefType,
   Scene,       // forms of type: SCEN
   ScriptVariableIndex,
   Sex,
   Shout,       // forms of type: SHOU
   Spell,       // forms of type: SPEL
   VariableIndex, // integer
   VariableName, // ???
   VATSFunction,
   VATSValue,
   Voicetype,   // forms of type: VTYP
   WardState,
   Weather,     // forms of type: WTHR
   Worldspace,  // forms of type: WRLD
};

struct ConditionFunction {
   private:
      typedef ConditionParamType _cpt;
   public:
      uint16_t    id = 0xFFFF;
      const char* name = "";
      const char* description = "";
      ConditionParamType paramTypes[3] = { ConditionParamType::None, ConditionParamType::None, ConditionParamType::None };
      bool        valid = true;
      //
      ConditionFunction(uint16_t id, const char* name, const char* d) : id(id), name(name), description(d) {};
      ConditionFunction(uint16_t id, const char* name, const char* d, _cpt a) : id(id), name(name), description(d), paramTypes{ a, _cpt::None, _cpt::None } {};
      ConditionFunction(uint16_t id, const char* name, const char* d, _cpt a, _cpt b) : id(id), name(name), description(d), paramTypes{ a, b, _cpt::None } {};
      ConditionFunction(uint16_t id, const char* name, const char* d, _cpt a, _cpt b, _cpt c) : id(id), name(name), description(d), paramTypes{ a, b, c } {};
      //
      enum class dummy_indicator { value };
      static constexpr dummy_indicator dummy = dummy_indicator::value;
      ConditionFunction(uint16_t id, dummy_indicator) : valid(false), id(id), name("Invalid Condition Function"), description("This condition ID is not valid.") {}
};

extern ConditionFunction conditionFunctions[]; // sequential array starting from 0
extern ConditionFunction extendedConditionFunctions[]; // use this array for discontiguous functions, typically script extender additions
extern const ConditionFunction* getConditionFunction(uint16_t id);

SCOPE_ENUM(ConditionTypeFlags, enum ConditionTypeFlags : uint8_t {
   or_linked    = 1,
   use_aliases  = 2,
   compare_to_global = 4,
   use_packdata = 8,
   swap_subject_and_target = 0x10,
});
enum class ConditionOperator {
   equal            = 0,
   not_equal        = 1,
   greater          = 2,
   greater_or_equal = 3,
   less             = 4,
   less_or_equal    = 5,
};

SCOPE_ENUM(ConditionRunOn, enum ConditionRunOn {
   subject       = 0,
   target        = 1,
   reference     = 2, SCOPED_ENUM_COMMENT("i.e. Condition::reference")
   combat_target = 3,
   linked_ref    = 4,
   quest_alias   = 5,
   package_data  = 6, SCOPED_ENUM_COMMENT("where does Condition store *which* packdata we're running on? param 3?")
   event_data    = 7,
});

struct Condition {
   uint8_t  type; // (ConditionTypeFlags << 5) | ConditionOperator
   float    compareToConstant;
   uint32_t compareToGlobalID;
   uint16_t function;
   uint32_t parameter1;
   uint32_t parameter2;
   uint32_t runOn;
   uint32_t reference; // only used for runOn == reference
   int32_t  parameter3 = -1; // if Run On == package_data, then this is the Package Data index (within the PACK containing this condition) to run on, and -1 means "NONE"
   std::string stringParam1;
   std::string stringParam2;
   //
   uint16_t eventFunction;
   uint16_t eventMember;
   uint32_t eventFormID;
   //
   inline ConditionOperator get_operator() const noexcept { return (ConditionOperator)((this->type >> 5) & 7); }
   inline uint8_t get_flags() const noexcept { return this->type & 0x1F; }
   //
   bool read(TESPluginRecord&); // assumes we've already opened a CTDA subrecord
   //
   void to_string(std::string& out) const;
};