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

   template<typename T, uint32_t count_per_block, uint32_t thread_count> class multithreaded_block_allocator {
      //
      // This is a variant on block_allocator that maintains multiple block lists. Each 
      // block list is owned by a single thread. The goal is to prevent threads from 
      // actually having to wait on each other, as would be the case if we had a single 
      // list of blocks and (by necessity, in that case) if we locked the entire 
      // allocator for the entire duration of a memory allocation.
      //
      // Once a thread terminates, its block list becomes available for another thread 
      // to claim. If more threads attempt to allocate elements than this allocator can 
      // keep track of, then the allocator falls back to malloc and free for the extra 
      // threads.
      //
      // Locking:
      //
      //  - When an allocation is requested, we use a shared lock while locating the 
      //    block list for the current thread. Once we've located the block list, we 
      //    unlock: we only allow a thread to access its own block list, so there's no 
      //    need for any further locking.
      //
      //  - When threads are being registered or unregistered, we use a non-shared lock.
      //
      //  - When a free is requested, we use a non-shared lock. A thread can potentially 
      //    be given a pointer to an element allocated by a different thread; ergo we 
      //    must search all block lists to see which contains the memory being freed.
      //
      // Usage example:
      //
      //    multithreaded_block_allocator<Foo, 1000, 4> allocator;
      //
      //    void task(void* state) {
      //       auto reg = allocator.register_thread();
      //       for(uint32_t i = 0; i < 100; i++) {
      //          new Foo;
      //          //
      //          // This would obviously leak, since we're not storing it anywhere 
      //          // and therefore can't free it later. In real code, you'd store it 
      //          // somewhere.
      //          //
      //       }
      //       // reg's destructor unregisters automatically, like a lock_guard
      //    }
      //
      //    void runAllTasks() {
      //       std::thread threads[5];
      //       for (uint32_t i = 0; i < std::extent<decltype(threads)>::value; i++)
      //          threads[i] = std::thread(task);
      //       for (uint32_t i = 0; i < std::extent<decltype(threads)>::value; i++)
      //          threads[i].join();
      //    }
      //
      public:
         typedef multithreaded_block_allocator<T, count_per_block, thread_count> my_type;
         typedef T element_type;
         static constexpr uint32_t element_size    = sizeof(T);
         static constexpr uint32_t count_per_block = count_per_block;
         //
         struct registration {
            my_type*        allocator;
            std::thread::id threadID;

            registration(my_type* a, std::thread::id id) : allocator(a), threadID(id) {};
            ~registration() {
               if (this->allocator) {
                  this->allocator->unregister_thread(this->threadID);
                  this->allocator = nullptr;
               }
            };
            registration(registration&& rhs) noexcept { // move constructor
               this->allocator = rhs.allocator;
               this->threadID  = rhs.threadID;
               rhs.allocator = nullptr;
            }
            registration(const registration& a) = delete; // do not allow copying

            inline operator bool() const noexcept { return this->allocator && this->threadID != std::thread::id(); }
         };
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
            #pragma warning(suppress: 26495) // buffer is uninitialized
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
         struct BlockList {
            std::thread::id thread;
            Block* first = new Block();
            //
            bool is_alive() {
               return this->thread != std::thread::id();
            }
         };
         //
         std::shared_mutex lock;
         BlockList lists[thread_count];
         //
         BlockList* _find_thread() noexcept {
            cobb::shared_lock_guard<std::shared_mutex> guard(this->lock);
            //
            auto id = std::this_thread::get_id();
            int32_t first_free = -1;
            for (uint32_t i = 0; i < std::extent<decltype(this->lists)>::value; i++) {
               if (this->lists[i].is_alive() && this->lists[i].thread == id)
                  return &this->lists[i];
            }
            return nullptr;
         }
         void unregister_thread(std::thread::id id) noexcept {
            std::lock_guard<std::shared_mutex> guard(this->lock);
            //
            for (uint32_t i = 0; i < std::extent<decltype(this->lists)>::value; i++) {
               auto& list = this->lists[i];
               if (list.thread == id) {
                  list.thread = std::thread::id();
                  return;
               }
            }
         }
         //
      public:
         registration register_thread() noexcept {
            std::lock_guard<std::shared_mutex> guard(this->lock);
            auto id = std::this_thread::get_id();
            //
            int32_t first_free = -1;
            for (uint32_t i = 0; i < std::extent<decltype(this->lists)>::value; i++) {
               if (!this->lists[i].is_alive()) {
                  first_free = i;
                  break;
               }
            }
            if (first_free >= 0) {
               this->lists[first_free].thread = id;
               return registration(this, id);
            }
            return registration(nullptr, std::thread::id());
         }
         void* allocate() {
            auto t = this->_find_thread(); // shared lock
            if (!t)
               return ::malloc(element_size);
            if (!t->first)
               t->first = new Block;
            Block* block = t->first;
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
            std::lock_guard<std::shared_mutex> guard(this->lock);
            //
            for (uint32_t i = 0; i < std::extent<decltype(this->lists)>::value; i++) {
               auto& t = this->lists[i];
               //
               Block* block = t.first;
               assert(block && "The block list doesn't have any blocks!");
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
                     if (block != t.first && block->info.presence.none()) { // never delete the first block in a list
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
            }
            ::free(mem);
         }
         void force_free_all() noexcept {
            std::lock_guard<std::shared_mutex> guard(this->lock);
            //
            for (uint32_t i = 0; i < std::extent<decltype(this->lists)>::value; i++) {
               auto& t = this->lists[i];
               //
               auto last = t.first;
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
                        std::ptrdiff_t addr  = start + (element_size * i);
                        //
                        auto element = (element_type*)addr;
                        element->~element_type();
                     }
                  }
                  presence.clear();
                  memset(last->buffer, 0, sizeof(last->buffer));
                  if (last != t.first) { // never delete the first block in a list
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
                  }
                  last = prev;
                  if (prev)
                     prev = prev->info.prev;
               } while (last);
            }
         }
         //
         void dumpStats() noexcept {
            std::lock_guard<std::shared_mutex> guard(this->lock);
            //
            printf("=================================================================================\n");
            printf("Dumping stats for this allocator...\n");
            printf("---------------------------------------------------------------------------------\n");
            for (uint32_t i = 0; i < std::extent<decltype(this->lists)>::value; i++) {
               auto& list = this->lists[i];
               printf("   Block list %d:\n", i);
               printf("---------------------------------------------------------------------------------\n");
               uint32_t blockCount = 0;
               uint32_t slotCount = 0;
               uint32_t slotsUsed = 0;
               uint32_t editorIDSizes = 0;
               for (auto block = list.first; block; block = block->info.next) {
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
                     }
                  }
               }
               printf("Blocks: %d\n", blockCount);
               printf("Total Slots: %d used out of %d\n", slotsUsed, slotCount);
               printf("Memory Usage:\n");
               printf(" - %d bytes overhead for block metadata\n", (sizeof(BlockInfo) * blockCount));
               printf(" - %d bytes allocated for elements\n", element_size * slotCount);
               printf(" - %d bytes in use for elements\n", element_size * slotsUsed);
               //
               printf("Overview by block:\n");
               blockCount = 0;
               for (auto block = list.first; block; block = block->info.next) {
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
               printf("---------------------------------------------------------------------------------\n");
            }
            printf("=================================================================================\n");
         }
   };
}