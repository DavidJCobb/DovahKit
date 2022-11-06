#pragma once
#include <optional>
#include "./_base.h"
#include "../enums/bool_operation.h"

namespace dovahkit::subsystems::worldinput::tools {
   class modify_camera_speed_flags : public base {
      public:
         static constexpr const char* function_name = "modify_camera_speed_flags";
         static constexpr compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };
      protected:
         modify_camera_speed_flags() { this->setup(this); }

      public:
         static modify_camera_speed_flags& get() {
            static modify_camera_speed_flags instance;
            return instance;
         }
      public:
         struct options {
            bool_operation boost     = bool_operation::no_op;
            bool_operation precision = bool_operation::no_op;
         };
         struct results {
            bool_operation boost     = bool_operation::no_op;
            bool_operation precision = bool_operation::no_op;

            void merge(const results& from);
         };
      public:
         virtual void invoke(const input_result&, const opaque_option_union&, combined_tool_results&) const override;

         virtual bool has_options() const { return true; }
   };
}