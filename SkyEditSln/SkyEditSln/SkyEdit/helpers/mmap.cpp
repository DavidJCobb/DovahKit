#include "mmap.h"
#include <winternl.h>
#include <windows.h>

namespace {
   uint32_t _get_alignment() {
      static uint32_t align = 0;
      if (align == 0) {
         _SYSTEM_INFO info;
         GetSystemInfo(&info);
         align = info.dwAllocationGranularity;
      }
      return align;
   }
}

#if defined(_WIN32) || defined(_WIN64)
   #include <io.h>
   #include <memoryapi.h>
   
   void* mmap(void* memory_addr, size_t length, int prot, int flags, FILE* file, off_t offset) {
      HMODULE hModule = LoadLibrary("ntdll.dll");
      typedef uint32_t(*f_RtlGetLastNtStatus)();
      auto RtlGetLastNtStatus = (f_RtlGetLastNtStatus)GetProcAddress(hModule, "RtlGetLastNtStatus");
      if (RtlGetLastNtStatus == nullptr) {
         printf("Error: could not find the function RtlGetLastNtStatus in library ntdll.dll.\n");
      }
      FreeLibrary(hModule);
      //
      if (length == 0) {
         errno = EINVAL;
         return MAP_FAILED;
      }
      uint32_t protect = 0;
      switch (prot) {
         //
         // TODO: the "private" option in Linux should map to "copy-on-write" in Windows
         //
         case PROT_READ:
            protect = PAGE_READONLY;
            break;
         case PROT_EXEC:
            protect = PAGE_EXECUTE;
            break;
         case PROT_WRITE:
         case PROT_READ | PROT_WRITE:
            protect = PAGE_READWRITE;
            break;
         case PROT_READ | PROT_EXEC:
            protect = PAGE_EXECUTE_READ;
            break;
         case PROT_WRITE | PROT_EXEC:
         case PROT_READ | PROT_WRITE | PROT_EXEC:
            protect = PAGE_EXECUTE_READWRITE;
            break;
      }
      {
         uint32_t misalign = offset % _get_alignment();
         if (offset >= misalign)
            offset -= misalign;
         else
            offset = 0;
      }
      uint64_t map_size = (uint64_t)length + offset;
      uint32_t high = map_size >> 0x20;
      uint32_t low  = map_size;
      //
      //
      HANDLE mapped_file = CreateFileMapping((HANDLE)_get_osfhandle(_fileno(file)), NULL, protect, high, low, NULL);
      if (mapped_file == NULL) {
         uint32_t nts = 0;
         if (RtlGetLastNtStatus)
            nts = RtlGetLastNtStatus();
         printf("<<Failed to create file mapping object. (%d/%08X)>>", GetLastError(), nts);
         return MAP_FAILED;
      }
      //
      if (sizeof(offset) == 8) {
         high = offset >> 0x20;
         low  = offset;
      } else {
         high = 0;
         low  = offset;
      }
      uint32_t access = 0;
      //
      // TODO: the "private" option in Linux should map to "copy-on-write" in Windows
      //
      if (prot & PROT_READ)
         access |= FILE_MAP_READ;
      if (prot & PROT_EXEC)
         access |= FILE_MAP_EXECUTE;
      //
      void* buffer = MapViewOfFile(mapped_file, access, high, low, length);
      if (buffer == NULL) {
         CloseHandle(mapped_file);
         uint32_t nts = 0;
         if (RtlGetLastNtStatus)
            nts = RtlGetLastNtStatus();
         printf("<<Failed to map view of file. (%d/%08X)>>", GetLastError(), nts);
         return MAP_FAILED;
      }
      CloseHandle(mapped_file);
      return buffer;
   }
   int munmap(void* view, size_t length) {
      return !UnmapViewOfFile(view);
   }
#endif

namespace cobb {
   bool mappable_pointer::map(FILE* file, off_t offset, size_t length, bool readonly) {
      HMODULE hModule = LoadLibrary("ntdll.dll");
      typedef uint32_t(*f_RtlGetLastNtStatus)();
      f_RtlGetLastNtStatus RtlGetLastNtStatus = nullptr;
      if (hModule) {
         RtlGetLastNtStatus = (f_RtlGetLastNtStatus)GetProcAddress(hModule, "RtlGetLastNtStatus");
         if (RtlGetLastNtStatus == nullptr) {
            printf("Error: could not find the function RtlGetLastNtStatus in library ntdll.dll.\n");
         }
         FreeLibrary(hModule);
      }
      //
      this->unmap();
      if (length == 0) {
         this->base_ptr = MAP_FAILED;
         return false;
      }
      uint32_t misalign = offset % _get_alignment();
      if (offset < misalign)
         misalign = offset;
      offset -= misalign;
      length += misalign;
      //
      uint64_t map_size = (uint64_t)length + offset;
      uint32_t high = map_size >> 0x20;
      uint32_t low  = map_size;
      //
      uint32_t protect = readonly ? PAGE_READONLY : PAGE_READWRITE;
      //
      HANDLE mapped_file = CreateFileMapping((HANDLE)_get_osfhandle(_fileno(file)), NULL, protect, high, low, NULL);
      if (mapped_file == NULL) {
         uint32_t nts = 0;
         if (RtlGetLastNtStatus)
            nts = RtlGetLastNtStatus();
         printf("<<Failed to create file mapping object. (%d/%08X)>>", GetLastError(), nts);
         this->base_ptr = MAP_FAILED;
         return false;
      }
      if (sizeof(offset) == 8) {
         high = offset >> 0x20;
         low = offset;
      } else {
         high = 0;
         low = offset;
      }
      uint32_t access = readonly ? FILE_MAP_READ : 0;
      void* buffer = MapViewOfFile(mapped_file, access, high, low, length);
      if (buffer == NULL) {
         uint32_t nts = 0;
         if (RtlGetLastNtStatus)
            nts = RtlGetLastNtStatus();
         CloseHandle(mapped_file);
         printf("<<Failed to map view of file. (%d/%08X)>>", GetLastError(), nts);
         this->base_ptr = MAP_FAILED;
         return false;
      }
      CloseHandle(mapped_file);
      this->base_ptr = buffer;
      this->base_length = length;
      this->data = (void*)((std::ptrdiff_t)this->base_ptr + misalign);
      return true;
   }
   void mappable_pointer::unmap() {
      if (this->base_ptr && this->base_ptr != MAP_FAILED) {
         UnmapViewOfFile(this->base_ptr);
         this->base_ptr = nullptr;
         this->data     = nullptr;
         this->base_length = 0;
      }
   }
   void mappable_pointer::dump(uint32_t cursor) {
      std::ptrdiff_t misalign = (std::ptrdiff_t)this->data - (std::ptrdiff_t)this->base_ptr;
      for (uint32_t i = 0; i < this->base_length; i++) {
         if (i == cursor) {
            printf("|");
         }
         if (i == misalign)
            printf("<<<");
         else if (i == this->base_length - misalign)
            printf(">>>");
         unsigned char* p = (unsigned char*)this->base_ptr;
         p += i;
         unsigned char c = *p;
         if (c > 0x1F && c < 0x7F) {
            printf("%c", c);
         } else {
            printf("%02X", c);
         }
         if (i + 1 != cursor)
            printf(" ");
      }
      printf("\n");
   }
   mappable_pointer::~mappable_pointer() {
      this->unmap();
   }
}