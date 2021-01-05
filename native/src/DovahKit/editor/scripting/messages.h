#pragma once
#include <string>

namespace editor_script {
   enum class message_type {
      none = 0,
      //
      // Main-to-script message types:
      //
      delete_form,
      //
      // Script-to-main message types:
      //
      log_text,               // Write text to the log.
      test_call_and_response, // A test of the editor script engine's ability to wait for the main thread to acknowledge a request.
   };

   class message {
      public:
         const message_type type = message_type::none;
         bool seen = false; // has this message been received and acknowledged by its recipient?
         
         message(message_type t) : type(t) {}
         
         //
         // Messages should always be heap-allocated.
         //
         // To send a non-blocking message, simply add it to the VM's message queue and 
         // then continue on; when the main thread acknowledges the message, it will be 
         // deleted automatically.
         //
         // To send a blocking message, add it to the VM's message queue and then loop 
         // until its (seen) property is set to (true); then, delete the message on your 
         // own. The VM checks the return value of (is_blocking) and will never auto-
         // delete any such message. Note that (is_blocking) could be called multiple 
         // times, and so should return a consistent value (no rand() shenanigans).
         //
         virtual bool is_blocking() const noexcept { return false; }
   };
}
