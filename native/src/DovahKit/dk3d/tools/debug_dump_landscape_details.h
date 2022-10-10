#pragma once
#include <QPoint>
#include "_base.h"
#include "../enums/pointer_position_type.h"
#include "../enums/sign.h"

namespace DK3D::tools {
   class debug_dump_landscape_details : public base {
      public:
         static constexpr const char* function_name = "debug_dump_landscape_details";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = true,
         };
      protected:
         debug_dump_landscape_details() { this->setup(this); }

      public:
         static debug_dump_landscape_details& get() {
            static debug_dump_landscape_details instance;
            return instance;
         }
      public:
         struct options {
            pointer_position_type position  = pointer_position_type::mouse;
         };
         struct results {
            bool exists = false;
            pointer_position_type position;
            QPointF mouse;
         };
      public:
         virtual void invoke(const input_result&, const opaque_option_union&, combined_tool_results&) const override;

         virtual bool has_options() const { return true; }
   };
}