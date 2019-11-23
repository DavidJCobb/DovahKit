#pragma once
#include <cstdint>
#include <string>
#include "../helpers/scoped_enum.h"
//
#include "conditions/arg_types.h"

class TESPluginRecord;

struct ConditionFunction {
   private:
      typedef ConditionParamType _cpt;
      typedef ConditionArgType& _cat;
   protected:
      ConditionArgType* argTypes[3] = { &ConditionArgTypes::None, &ConditionArgTypes::None, &ConditionArgTypes::None }; // aRrAy Of ReFeReNcE iS nOt AlLoWeD
   public:
      uint16_t    id = 0xFFFF;
      const char* name = "";
      const char* description = "";
      bool        valid = true;
      //
      ConditionFunction(uint16_t id, const char* name, const char* d) : id(id), name(name), description(d) {};
      ConditionFunction(uint16_t id, const char* name, const char* d, _cat a) : id(id), name(name), description(d), argTypes{ &a, &ConditionArgTypes::None, &ConditionArgTypes::None } {};
      ConditionFunction(uint16_t id, const char* name, const char* d, _cat a, _cat b) : id(id), name(name), description(d), argTypes{ &a, &b, &ConditionArgTypes::None } {};
      ConditionFunction(uint16_t id, const char* name, const char* d, _cat a, _cat b, _cat c) : id(id), name(name), description(d), argTypes{ &a, &b, &c } {};
      //
      ConditionArgType* getArgumentType(uint8_t index) const noexcept;
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