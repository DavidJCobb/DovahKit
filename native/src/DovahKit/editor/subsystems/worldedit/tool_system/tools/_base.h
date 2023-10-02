#pragma once
#include <concepts>
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"
#include "helpers/bitstreams/round_trip_test.h"
#include "helpers/bitstreams/round_trip_test_with_targeted_scramble.h"
#include "helpers/eight_cc.h"
#include "helpers/enum_flags.h"
#include "editor/subsystems/worldinput/tool_request_cause.h"
#include "../../enums/editor_mode.h"

namespace dovahkit::subsystems::worldedit {
   class tool_response_tuple;

   namespace tools {
      class opaque_options_union;

      using editor_mode_set = cobb::enum_flags<editor_mode, 3>;
      inline constexpr editor_mode_set all_editor_modes = editor_mode_set::with_all_set();
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   struct compile_time_tool_options {
      bool use_strict_ordering = false; // set to (true) if the order of results within a single frame matters when merging results
   };

   class _base {
      public:
         using tool_request_cause = dovahkit::subsystems::worldinput::tool_request_cause;

      public:
         static constexpr const char*          function_name = "unnamed";
         static constexpr const cobb::eight_cc function_code = 0;

         static constexpr const compile_time_tool_options compile_time_options = {};

         static constexpr const bool is_raycast_sensitive = false;
         static constexpr const editor_mode_set supported_editor_modes = all_editor_modes;

      public:
         // Subclasses should override this if they have options to offer.
         struct options {
            constexpr void stream(cobb::bitstreams::reader&) {}
            constexpr void stream(cobb::bitstreams::writer&) const {}

            constexpr bool operator==(const options&) const = default;
         };
   };

   template<typename T> concept tool_with_options_member_type = requires { typename T::options; requires std::is_base_of_v<_base, T>; };
   template<typename T> concept tool_sans_options_member_type = !tool_with_options_member_type<T> && std::is_base_of_v<_base, T>;

   template<typename T> concept tool_with_response_member_type = requires { typename T::response; requires std::is_base_of_v<_base, T>; };
   template<typename T> concept tool_sans_response_member_type = !tool_with_response_member_type<T> && std::is_base_of_v<_base, T>;
}