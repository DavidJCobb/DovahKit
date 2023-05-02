#pragma once
#include "./concepts/tool_or_tool_options.h"
#include "./tool_id.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<tool_or_tool_options T>
   constexpr const tool_id id_of = []() -> tool_id {
      size_t i = all_tools::index_of_matching_type<[]<typename Current>() {
         if constexpr (tool_with_options_member_type<Current>) {
            if constexpr (std::is_same_v<T, typename Current::options>)
               return true;
         }
         return std::is_same_v<T, Current>;
      }>;
      return (tool_id)i;
   }();
}