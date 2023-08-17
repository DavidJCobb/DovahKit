#pragma once

namespace cobb::ini {
   // If set to true, static assertions will be preferred for data correctness checks. These will 
   // produce relatively informative error messages when attempting to compile, but are unlikely to 
   // show any indication of any errors in IntelliSense. If set to false, concepts will be preferred 
   // for data correctness checks. These are more likely to show errors in IntelliSense, but those 
   // errors will generally have unintuitive messages (e.g. claiming that no instance of a function 
   // template matches the provided arguments, when in reality an NTTP has invalid data and fails to 
   // meet the template function's constraints).
   constexpr const bool prefer_static_assertions = false;
}