#include "floating_point_literal.h"
#include "integer_literal.h"

#include <cstdint>

namespace {
   #pragma region Int tests
   static_assert(cobb::integer_literal<unsigned int>("1234").get_value() == 1234);
   static_assert(cobb::integer_literal<int>("-1234").get_value() == -1234);

   static_assert(cobb::integer_literal<int8_t>("-128").get_value() == -128);
   static_assert(cobb::integer_literal<int8_t>("128").get_error() == cobb::integer_literal<int8_t>::error::overflow);
   #pragma endregion

   #pragma region Float tests
   static_assert(cobb::floating_point_literal<float>("123").within_epsilon_of(123));
   static_assert(cobb::floating_point_literal<float>("123.0").within_epsilon_of(123));

   static_assert(cobb::floating_point_literal<float>("123.4").within_epsilon_of(123.4));
   static_assert(cobb::floating_point_literal<float>("123.40").within_epsilon_of(123.4));
   static_assert(cobb::floating_point_literal<float>("123.400").within_epsilon_of(123.4));

   static_assert(cobb::floating_point_literal<float>("1234E-1").within_epsilon_of(123.4));

   static_assert(cobb::floating_point_literal<float>("1234E1").within_epsilon_of(12340));
   static_assert(cobb::floating_point_literal<float>("1234E+1").within_epsilon_of(12340));

   static_assert(cobb::floating_point_literal<float>("-1234E1").within_epsilon_of(-12340));

   static_assert(cobb::floating_point_literal<float>("123.").get_error().has_value() == false);
   static_assert(cobb::floating_point_literal<float>(".123").get_error().has_value() == false);
   static_assert(cobb::floating_point_literal<float>("-.123").get_error().has_value() == false);
   static_assert(cobb::floating_point_literal<float>("-").get_error() == cobb::floating_point_literal<float>::error::no_digits); // no digits
   static_assert(cobb::floating_point_literal<float>(".").get_error() == cobb::floating_point_literal<float>::error::no_digits); // no digits

   static_assert(cobb::floating_point_literal<float>("..").get_error() == cobb::floating_point_literal<float>::error::invalid); // too many decimal separators
   static_assert(cobb::floating_point_literal<float>(".123.").get_error() == cobb::floating_point_literal<float>::error::invalid); // too many decimal separators

   static_assert(cobb::floating_point_literal<float>("1.2E1.2").get_error() == cobb::floating_point_literal<float>::error::invalid); // exponent cannot have a decimal point of its own
   #pragma endregion
}