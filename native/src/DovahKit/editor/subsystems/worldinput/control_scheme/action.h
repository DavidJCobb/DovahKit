#pragma once
#include <QString>
#include "../enums/button_press_type.h"
#include "../input_sequence.h"
#include "editor/subsystems/worldedit/tool_system/tool_id.h"

namespace cobb::bitstreams {
   class reader;
   class writer;
}
namespace dovahkit::subsystems::worldedit::tools {
   class opaque_options_union;
}

namespace dovahkit::subsystems::worldinput {
   class control_scheme_action {
      public:
         static constexpr const size_t max_name_length = 1023;

      public:
         QString name;
         typename input_sequence    input_sequence;
         typename button_press_type button_press_type = button_press_type::press;
         struct {
            worldedit::tools::tool_id               id = worldedit::tools::id_of_none;
            worldedit::tools::opaque_options_union* options = nullptr;
         } tool;

         bool operator==(const control_scheme_action&) const noexcept;

         void stream(cobb::bitstreams::reader&);
         void stream(cobb::bitstreams::writer&) const;
   };
}