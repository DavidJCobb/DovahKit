#pragma once
#include <mutex>

namespace dovah {
   //
   // This class exists to work around the fact that `std::terminate` in MSVC has 
   // totally non-standard behavior: programs are supposed to have just one global 
   // terminate handler, but MSVC instead makes the terminate handler exist as per-
   // thread state. There's no way to set a global terminate handler, e.g. for the 
   // purpose of reporting crashes to a program's developer.
   // 
   // As such, the best we can do is have DovahKit's backend use a shared terminate 
   // handler, and expose customization to frontends.
   // 
   // To keep the implementation simple and avoid performance impacts, this behavior 
   // is not retroactive. You must set the handler on this singleton before you run 
   // backend code; any previously-spawned worker threads will not see the change to 
   // the desired terminate handler.
   //
   class worker_thread_termination_handler {
      public:
         using handler_type = void(*)();

      protected:
         worker_thread_termination_handler() {}
         ~worker_thread_termination_handler() {}

      public:
         static worker_thread_termination_handler& get();

         handler_type get_handler() const;
         void set_handler(handler_type);

         static void update_this_thread();

      protected:
         handler_type       _handler;
         mutable std::mutex _mutex;
   };
}
