#pragma once

namespace dovahkit::subsystems::worldedit::tools {
   template<typename Tool> concept tool_with_response = requires {
      typename Tool::response;
   };
}