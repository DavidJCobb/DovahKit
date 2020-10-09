#pragma once
#include <string>

namespace cobb::windows {
   extern void expand_environment_variables(std::string&);
   extern void expand_environment_variables(std::wstring&);
}