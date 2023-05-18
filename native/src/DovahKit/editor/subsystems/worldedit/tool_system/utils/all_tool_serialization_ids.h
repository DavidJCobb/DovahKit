#pragma once
#include "helpers/eight_cc.h"
#include "../tools/_all.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr const auto all_tool_serialization_ids = [](){
      std::array<cobb::eight_cc, all_tools::count> entries = {};
      {
         size_t i = 0;
         all_tools::for_each([&entries, &i]<typename Tool>() {
            entries[i] = Tool::function_code;
            ++i;
         });
      }
      return entries;
   }();
}