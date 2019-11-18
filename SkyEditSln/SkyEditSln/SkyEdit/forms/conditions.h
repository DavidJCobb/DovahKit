#pragma once
#include <cstdint>
#include <string>

class TESPluginRecord;

namespace condition_arg_type {
   enum class advance_action {
      normal,
      power_attack,
      bash,
      lockpick_success,
      lockpick_broken,
   };
   enum class alignment { // karma
      good,
      neutral,
      evil,
      very_good,
      very_evil,
   };
   enum class axis : unsigned char {
      x = 'X',
      y = 'Y',
      z = 'Z',
   };
   enum class critical_stage {
      none,
      goo_start,
      goo_end,
      disintegrate_start,
      disintegrate_end,
   };
   enum class sex : uint32_t {
      male,
      female,
   };
}

enum class ConditionParamType : uint8_t {
   None = 0,
   Actor,      // forms of type: ACHR
   ActorBase,  // forms of type: NPC_
   ActorValue,
   Alias,
   Alignment,  // enum; karma
   Axis,       // char
   BaseForm,
   Cell,       // forms of type: CELL
   Class,      // forms of type: CLAS
   CrimeType,
   CriticalStage, // enum
   EquipType,
   Faction,    // forms of type: FACT
   Float,
   FormType,
   Furniture,  // forms of type: FURN
   Global,     // forms of type: GLOB
   Integer,
   InventoryItem, // forms of type: [todo; anything that can ever go in an inventory]
   Keyword,    // forms of type: KYWD
   MiscStat,
   Quest,      // forms of type: QUST
   QuestStage,
   Package,    // forms of type: PACK
   Race,       // forms of type: RACE
   ObjectReference, // forms of type: ACHR, REFR
   ScriptVariableIndex,
   Sex,
   VariableIndex, // integer
   Voicetype, // forms of type: VTYP
   Weather,   // forms of type: WTHR
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

extern ConditionFunction conditionFunctions[];
extern const ConditionFunction* getConditionFunction(uint16_t id);

struct ConditionTypeFlags { // scoped enum with implicit casting to int
   ConditionTypeFlags() = delete;
   enum : uint8_t {
      or_linked    = 1,
      use_aliases  = 2,
      compare_to_global = 4,
      use_packdata = 8,
      swap_subject_and_target = 0x10,
   };
};
enum class ConditionOperator {
   equal = 0,
   not_equal = 1,
   greater   = 2,
   greater_or_equal = 3,
   less      = 4,
   less_or_equal = 5,
};

struct ConditionRunOn { // scoped enum with implicit casting to int
   ConditionRunOn() = delete;
   enum {
      subject       = 0,
      target        = 1,
      reference     = 2,
      combat_target = 3,
      linked_ref    = 4,
      quest_alias   = 5,
      package_data  = 6,
      event_data    = 7,
   };
};

struct Condition {
   uint8_t  type; // (ConditionTypeFlags << 5) | ConditionOperator
   float    compareToConstant;
   uint32_t compareToGlobalID;
   uint16_t function;
   uint32_t parameter1;
   uint32_t parameter2;
   uint32_t runOn;
   uint32_t reference; // only used for runOn == reference
   int32_t  parameter3;
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