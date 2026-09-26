#pragma once
#include <atomic>
#include <thread>
#include <vector>

namespace dovah {
   class bsa_archive;
   class bsa_load_order;

   class bsa_threaded_reader {
      protected:
         bsa_load_order& owner;
         std::thread     thread;
         std::vector<bsa_archive*> archives;
         std::atomic<bool> complete = false;
         
         static void _exec(bsa_threaded_reader*);
         
      public:
         bsa_threaded_reader(bsa_load_order&);
         
         void add_archive(bsa_archive*);
         
         void start();
         void wait_for();
         inline bool is_complete() const noexcept { return this->complete; }
   };
}