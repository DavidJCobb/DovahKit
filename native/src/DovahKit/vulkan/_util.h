#pragma once

namespace vulkanDK {
   class no_copy {
      public:
         no_copy() {}
         no_copy(no_copy&&) = default;

      private:
         no_copy(const no_copy&) = delete;
         no_copy& operator=(const no_copy&) = delete;
   };

   class only_heap_allocate {
      public:
         only_heap_allocate() {}
      protected:
         ~only_heap_allocate() {}
   };
}