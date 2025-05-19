#pragma once
#include <memory>
#include "editor/subsystems/worldedit/tool_system/options_union.h"
#include "editor/subsystems/worldedit/tool_system/tool_id.h"

namespace dovahkit::subsystems::worldinput::util {
   class tool_binding {
      protected:
         using tool_id         = worldedit::tools::tool_id;
         using options_type    = worldedit::tools::options_union;
         using options_pointer = std::unique_ptr<options_type>;

      public:
         tool_binding() {}
         tool_binding(const tool_binding&);
         tool_binding(tool_binding&&) noexcept;

         explicit tool_binding(tool_id id, options_type* opt) : id(id) {
            this->options.reset(opt);
         }

         tool_binding& operator=(const tool_binding&);
         tool_binding& operator=(tool_binding&&) noexcept;

         bool operator==(const tool_binding& other) const;

         bool compare_fast_fields(const tool_binding&) const;
         bool compare_slow_fields(const tool_binding&) const;

      public:
         tool_id         id      = worldedit::tools::id_of_none;
         options_pointer options = nullptr;
   };
}