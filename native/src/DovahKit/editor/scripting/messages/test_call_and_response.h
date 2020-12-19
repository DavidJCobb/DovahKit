#pragma once
#include "../messages.h"

namespace editor_script::messages {
   class test_call_and_response : public message {
      //
      // Test message intended for use by DovahKit's developer only. It's a dummy message with 
      // no data, set up to be blocking; the main thread must handle it in order for script 
      // execution to continue.
      //
      public:
         test_call_and_response() : message(message_type::test_call_and_response) {}
         //
         virtual bool is_blocking() const noexcept override { return true; }
   };
}
