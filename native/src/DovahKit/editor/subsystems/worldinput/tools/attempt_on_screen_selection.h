#pragma once
#include <QPoint>
#include "./_base.h"
#include "../enums/selection_operation.h"
#include "../enums/pointer_position_type.h"
#include "../enums/sign.h"

namespace dovahkit::subsystems::worldinput::tools {
   class attempt_on_screen_selection : public base {
      public:
         static constexpr const char* function_name = "attempt_on_screen_selection";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = true,
         };
      protected:
         attempt_on_screen_selection() { this->setup(this); }

      public:
         static attempt_on_screen_selection& get() {
            static attempt_on_screen_selection instance;
            return instance;
         }
      public:
         struct options {
            selection_operation   operation = selection_operation::add;
            pointer_position_type position  = pointer_position_type::mouse;
         };
         struct results {
            selection_operation   operation;
            pointer_position_type position;
            //
            QPointF mouse;
            bool sweep = false; // for any non-button inputs; sweep the pointer over objects to modify selection state; Worldedit must track when an object is swept over/out
         };
      public:
         virtual void invoke(const input_result&, const opaque_option_union&, combined_tool_results&) const override;

         virtual bool has_options() const { return true; }
   };
}