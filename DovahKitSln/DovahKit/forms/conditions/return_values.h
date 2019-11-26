#pragma once

struct ConditionReturnValue {
   const char* const name;
   const float       value;
   //
   ConditionReturnValue(const char* a, float b) : name(a), value(b) {}
};