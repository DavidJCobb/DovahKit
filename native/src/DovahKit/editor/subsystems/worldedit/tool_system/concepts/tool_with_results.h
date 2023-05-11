#pragma once

namespace dovahkit::subsystems::worldedit::tools {
   template<typename Tool> concept tool_with_results = requires {
      typename Tool::results;
   };
}