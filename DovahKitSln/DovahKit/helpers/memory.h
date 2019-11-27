#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <shared_mutex>
#include "bitset.h" // block_allocator
#include "locks.h"
#include "threading.h"

namespace cobb {
   class generic_buffer {
      private:
         void*    _data = nullptr;
         uint32_t _size = 0;
         uint32_t _capacity = 0;
      public:
         void allocate(uint32_t bytes);
         void free(); // checks whether (data) is nullptr
         inline void* raw() { return this->_data; }
         void shrink_to_fit();

         inline uint32_t size() const noexcept { return this->_size; }
         inline uint32_t capacity() const noexcept { return this->_capacity; }
         inline bool empty() const noexcept {
            return this->_data == nullptr && !this->_size;
         }

         inline void* operator->() { return this->_data; }
         inline void* operator*() { return this->_data; }

         inline operator char*() { return (char*)this->_data; }
         inline operator void*() { return this->_data; }
         explicit inline operator std::ptrdiff_t() const { return (std::ptrdiff_t)this->_data; }

         generic_buffer() {};
         generic_buffer(uint32_t bytes) { this->allocate(bytes); }
         ~generic_buffer() {
            this->free();
         }
   };

   struct block_allocator_debug_printer {
      //
      // A struct that can be subclassed and then passed to block_allocator::dumpStats, to 
      // log the full state of an allocator. The goal is to allow the display of custom 
      // information about the allocated objects, e.g. the total size of resources they may 
      // own that are allocated on the normal heap.
      //
      virtual void forBlock(uint32_t index) = 0;
      virtual void forElement(void* element) = 0;
      virtual void printExtraStats() = 0;
   };
   template<typename T, uint32_t count_per_block> class block_allocator {
      //
      // A custom allocator that allocates in blocks, to reduce memory fragmentation and 
      // overhead (i.e. every heap allocation has to track the size allocated and some 
      // other metadata; there's no point in doing that for each individual instance). 
      // This isn't faster or slower than the normal new/delete.
      //
      // Note that this allocator is not thread-safe. If you need thread-safe allocation, 
      // use the standard new/delete operators, or use the multithreaded_block_allocator 
      // defined below.
      //
      // A typical usage example:
      //
      //    struct MyObj {
      //       static void* operator new(std::size_t sz);
      //       static void operator delete(void* ptr, std::size_t sz);
      //    }
      //    class MyObjHeap : public block_allocator<MyObj, 100> {
      //       public:
      //          inline static MyObjHeap& get() {
      //             static MyObjHeap instance;
      //             return instance;
      //    }
      //
      //    void* MyObj::operator new(std::size_t sz) {
      //       if (sz != sizeof(MyObj))
      //          return ::operator new(sz);
      //       return MyObjHeap::get().allocate();
      //    }
      //    void MyObj::operator delete(void* ptr, std::size_t sz) {
      //       if (sz != sizeof(MyObj))
      //          return ::operator delete(ptr, sz);
      //       return MyObjHeap::get().free(ptr);
      //    }
      //
      public:
         typedef T element_type;
         static constexpr uint32_t element_size    = sizeof(T);
         static constexpr uint32_t count_per_block = count_per_block;
         //
      protected:
         struct Block;
         struct BlockInfo {
            Block* prev = nullptr;
            Block* next = nullptr;
            cobb::bitset<count_per_block> presence;
         };
         struct Block {
            BlockInfo info;
            uint8_t   buffer[count_per_block * element_size];
            //
            void* allocate() {
               auto i = this->info.presence.find_first_clear();
               if (i < 0)
                  return nullptr;
               std::ptrdiff_t start = (std::ptrdiff_t) & this->buffer;
               std::ptrdiff_t addr = start + (element_size * i);
               this->info.presence.set(i);
               return (void*)addr;
            }
         };
      public:
         Block* firstBlock = nullptr;
         //
         void* allocate() {
            if (!this->firstBlock)
               this->firstBlock = new Block;
            Block* block = this->firstBlock;
            Block* last = block;
            void* out = block->allocate();
            while (!out) {
               block = block->info.next;
               if (block)
                  last = block;
               else
                  break;
               out = block->allocate();
            }
            if (out)
               return out;
            if (!block) {
               assert(last && "Couldn't figure out how to create a new block.");
               auto next = new Block;
               last->info.next = next;
               next->info.prev = last;
               out = next->allocate();
            }
            return out;
         }
         void free(void* mem) {
            assert(this->firstBlock && "Cannot free; nothing was allocated.");
            Block* block = this->firstBlock;
            do {
               std::ptrdiff_t m_addr  = (std::ptrdiff_t)mem;
               std::ptrdiff_t b_start = (std::ptrdiff_t) & block->buffer;
               std::ptrdiff_t b_end   = b_start + sizeof(block->buffer);
               if (m_addr >= b_start && m_addr < b_end) {
                  m_addr -= b_start;
                  uint16_t index = m_addr / sizeof(element_type);
                  assert(m_addr % element_size == 0       && "Cannot free; element is not aligned.");
                  assert(block->info.presence.test(index) && "You're freeing something that was already free!");
                  block->info.presence.reset(index);
                  //
                  if (block != this->firstBlock && block->info.presence.none()) {
                     //
                     // This block is no longer in use. Delete it.
                     //
                     auto p = block->info.prev;
                     auto n = block->info.next;
                     if (p)
                        p->info.next = n;
                     if (n)
                        n->info.prev = p;
                     delete block;
                  }
                  return;
               }
            } while (block = block->info.next);
            assert(false && "Cannot free; element not found on our heap.");
         }
         //
         void dumpStats(block_allocator_debug_printer& printer) {
            printf("=================================================================================\n");
            printf("Dumping stats for this allocator...\n");
            uint32_t blockCount = 0;
            uint32_t slotCount  = 0;
            uint32_t slotsUsed  = 0;
            uint32_t editorIDSizes = 0;
            for (auto block = this->firstBlock; block; block = block->info.next) {
               printer.forBlock(blockCount);
               blockCount++;
               slotCount += count_per_block;
               //
               auto& presence = block->info.presence;
               for (uint16_t i = 0; i < count_per_block; i++) {
                  if (presence.test(i)) {
                     slotsUsed++;
                     //
                     std::ptrdiff_t addr = (std::ptrdiff_t)block->buffer + element_size * i;
                     T* element = (T*)addr;
                     printer.forElement(element);
                  }
               }
            }
            printf("Blocks: %d\n", blockCount);
            printf("Total Slots: %d used out of %d\n", slotsUsed, slotCount);
            printf("Memory Usage:\n");
            printf(" - %d bytes overhead for block metadata\n", (sizeof(BlockInfo) * blockCount));
            printf(" - %d bytes allocated for elements\n", element_size * slotCount);
            printf(" - %d bytes in use for elements\n",  element_size * slotsUsed);
            printer.printExtraStats();
            //
            if (count_per_block < 150) {
               printf("Overview by block:\n");
               blockCount = 0;
               for (auto block = this->firstBlock; block; block = block->info.next) {
                  printf(" - Block %d:\n", blockCount);
                  blockCount++;
                  //
                  auto& presence = block->info.presence;
                  printf("    - ");
                  for (uint16_t i = 0; i < count_per_block; i++) {
                     if (presence.test(i))
                        printf("1");
                     else
                        printf("0");
                  }
                  printf("\n");
               }
               printf("All blocks listed.\n");
            }
            printf("=================================================================================\n");
         }
         void force_free_all() {
            auto last = this->firstBlock;
            if (!last)
               return;
            while (last->info.next)
               last = last->info.next;
            //
            auto prev = last->info.prev;
            do {
               auto& presence = last->info.presence;
               for (uint32_t i = 0; i < count_per_block; i++) {
                  if (presence.test(i)) {
                     std::ptrdiff_t start = (std::ptrdiff_t) & last->buffer;
                     std::ptrdiff_t addr = start + (element_size * i);
                     //
                     auto element = (element_type*)addr;
                     element->~element_type();
                  }
               }
               presence.clear();
               memset(last->buffer, 0, sizeof(last->buffer));
               //
               // This block is no longer in use. Delete it.
               //
               auto p = last->info.prev;
               auto n = last->info.next;
               if (p)
                  p->info.next = n;
               if (n)
                  n->info.prev = p;
               delete last;
               //
               last = prev;
               if (prev)
                  prev = prev->info.prev;
            } while (last);
         }
   };
}