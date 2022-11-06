#pragma once
#include "./_base.h"

namespace dovahkit::subsystems::worldinput::tools {
   class debug_log : public base {
      public:
         static constexpr const char* function_name = "debug_log";
      protected:
         debug_log() { this->setup(this); }

      public:
         static debug_log& get() {
            static debug_log instance;
            return instance;
         }
      public:
         struct options {
            int number = 0;
         };
      public:
         virtual void invoke(const input_result&, const opaque_option_union&, combined_tool_results&) const override;

         virtual bool has_options() const { return true; }
   };
}