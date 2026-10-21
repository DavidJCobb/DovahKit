#pragma once
#include <atomic>
#include <thread>
#include <vector>
namespace dovah::bsa {
   class archive;
   class archive_collection;
}

namespace dovah::bsa {
   class thread_for_parsing {
      protected:
         archive_collection&   owner;
         std::thread           thread;
         std::vector<archive*> archives;
         std::atomic<bool>     complete = false;
         
         static void _exec(thread_for_parsing*);
         
      public:
         thread_for_parsing(archive_collection&);
         
         void add_archive(archive&);
         
         void start();
         void wait_for();
         constexpr bool is_complete() const noexcept { return this->complete; }
   };
}