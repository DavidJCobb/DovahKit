#pragma once
#include <cstdint>
#include <string>
#include "../helpers/scoped_enum.h"
//
#include "conditions/arg_types.h"

class FormStub;
class TESPluginRecord;

struct ConditionFunction {
   public:
      //
      // Dummy classes, for constructors:
      //
      enum class sentinel_is_event {};
      static constexpr sentinel_is_event uses_event_data = sentinel_is_event();
   private:
      typedef ConditionArgType& _cat;
   public:
      uint16_t    id = 0xFFFF;
      const char* name = "";
      const char* description = "";
      const bool  valid = true;
      const bool  usesEventData = false;
      ConditionArgType* const argTypes[2] = { &ConditionArgTypes::None, &ConditionArgTypes::None }; // aRrAy Of ReFeReNcE iS nOt AlLoWeD
      //
      ConditionFunction(uint16_t id, const char* name, const char* d) : id(id), name(name), description(d) {};
      ConditionFunction(uint16_t id, const char* name, const char* d, _cat a) : id(id), name(name), description(d), argTypes{ &a, &ConditionArgTypes::None } {};
      ConditionFunction(uint16_t id, const char* name, const char* d, _cat a, _cat b) : id(id), name(name), description(d), argTypes{ &a, &b } {};
      //
      ConditionFunction(uint16_t id, const char* name, const char* d, sentinel_is_event) : id(id), name(name), description(d), usesEventData(true) {};
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

SCOPE_ENUM(ConditionRunOn, enum ConditionRunOn : uint32_t {
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
   uint8_t   type; // (ConditionTypeFlags << 5) | ConditionOperator
   float     compareToConstant;
   form_id_t compareToGlobalID;
   uint16_t  function;
   ConditionArgValue parameters[2];
   ConditionRunOn runOn;
   form_id_t      runOnRef; // only used for runOn == reference
   int32_t        runOnIndex = -1; // xEdit calls this "Parameter 3" // if Run On == package_data, then this is the Package Data index (within the PACK containing this condition) to run on, and -1 means "NONE"
   //
   uint16_t eventFunction;
   uint16_t eventMember;
   uint32_t eventFormID;
   //
   ConditionArgType* getArgumentType(uint8_t index) const noexcept;
   ConditionArgUnderlyingType getArgumentUnderlyingType(uint8_t index) const noexcept;
   //
   inline ConditionOperator get_operator() const noexcept { return (ConditionOperator)((this->type >> 5) & 7); }
   inline uint8_t get_flags() const noexcept { return this->type & 0x1F; }
   //
   bool read(TESPluginRecord&); // assumes we've already opened a CTDA subrecord
   static void generateUseInfo(TESPluginRecord&, FormStub*);
   //
   void to_string(std::string& out) const;
};