#pragma once
#include <cstddef>
#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
   #include <stdio.h>
   #include <sys/types.h>

   #define MAP_FAILED ((void*)-1)

   #define PROT_READ  1
   #define PROT_WRITE 2
   #define PROT_EXEC  4
   #define PROT_NONE  0

   #define MAP_SHARED  1
   #define MAP_PRIVATE 2
   #define MAP_SHARED_VALIDATE 3
   #define MAP_ANONYMOUS 16
   
   void* mmap(void* memory_addr, size_t length, int prot, int flags, FILE* file, off_t offset);
   int   munmap(void* memory_addr, size_t length);
#endif

namespace cobb {
   class mappable_pointer {
      //
      // This class manages the mapping of part of a file into memory via Windows APIs. 
      // Mapped file views must have a CPU-dependent alignment; this class handles that 
      // for you and ensures that the region you want to read is fully mapped.
      //
      // That said, Windows' file-mapping APIs (their counterpart to mmap) aren't good 
      // for especially large files. You can't map the middle of a file into memory; 
      // you must map everything from the start of the file to the end of the region 
      // you wish to actually use.
      //
      public:
         void* data = nullptr;
      private:
         void*    base_ptr    = nullptr;
         uint32_t base_length = 0;
         //
      public:
         bool map(FILE* file, off_t offset, size_t length, bool readonly = true);
         void unmap();

         inline void* operator->() { return this->data; };
         inline void* operator*() { return this->data; };

         inline operator bool() { return this->base_ptr != nullptr; }
         inline operator void*() { return this->data; }
         explicit inline operator std::ptrdiff_t() { return (std::ptrdiff_t)this->data; }

         void dump(uint32_t cursor);

         mappable_pointer& operator=(void* p) {
            this->unmap();
            this->data = p;
            return *this;
         }

         ~mappable_pointer();
   };
}