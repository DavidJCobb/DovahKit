#pragma once
#include <array>
#include "helpers/function_pointer.h"
#include "./tools/_all.h"
#include "./tool_id.h"

namespace dovahkit::subsystems::worldedit {
   class tool_response_tuple;
}
namespace dovahkit::subsystems::worldinput {
   struct tool_request_cause;
}

namespace dovahkit::subsystems::worldedit::tools {
   struct tool_dispatch_table_entry {
      using invoke_handler_t         = void(const worldinput::tool_request_cause&, const options_union&, tool_response_tuple&);
      using invoke_hold_up_handler_t = void(const options_union&, tool_response_tuple&);

      const char* name = nullptr;
      cobb::function_pointer<invoke_handler_t>         request              = nullptr;
      cobb::function_pointer<invoke_hold_up_handler_t> request_hold_release = nullptr;
   };

   constexpr const auto tool_dispatch_table = []() {
      std::array<tool_dispatch_table_entry, all_tools::count> table = {};

      size_t i = 0;
      all_tools::for_each([&table, &i]<typename Tool>() {
         auto& item = table[i];
         item.name                 = Tool::function_name;
         item.request              = &Tool::request;
         item.request_hold_release = &Tool::request_for_hold_release;
         //
         ++i;
      });

      return table;
   }();
}