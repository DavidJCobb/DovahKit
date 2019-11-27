#pragma once
#include <cassert>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>
#include "../helpers/bitset.h"

//
// An attempt at reimplementing cobb::multithreaded_block_allocator, inspired by a Qt 
// blog post that used a thread_local unique_ptr instead of manually registering threads; 
// if we can get such a thing working, then our heap can be strictly internal, which 
// eliminates the need for an externally-accessible singleton and thereby allows use of 
// the heap with STL containers that take Allocators and rebind them.
//

namespace cobb_ex {
   template<typename T, uint32_t count_per_block> class multiheap {
      public:
         using mapped_type = T;
         static constexpr uint32_t element_size = sizeof(T);
         static constexpr uint32_t count_per_block = count_per_block;
         //
      protected:
         struct Block;
         struct BlockInfo {
            Block* prev = nullptr;
            Block* next = nullptr;
            uint32_t remaining = count_per_block; // optimization for large block sizes
            uint32_t startFrom = 0; // optimization for large block sizes
            cobb::bitset<count_per_block> presence;
            //
            inline void on_allocate(uint32_t index) noexcept {
               this->presence.set(index);
               this->remaining--;
               this->startFrom = index;
            }
            inline void on_free(uint32_t index) noexcept {
               this->presence.reset(index);
               this->remaining++;
               if (this->startFrom > index)
                  this->startFrom = index;
            }
         };
         struct Block {
            BlockInfo info;
            uint8_t   buffer[count_per_block * element_size];
            //
            inline bool has_free_slots()     const noexcept { return this->info.remaining; };
            inline bool has_any_slots_used() const noexcept {
               //return !this->info.presence.none();
               return this->info.remaining < count_per_block;
            }
            void* try_allocate() noexcept {
               if (!this->has_free_slots())
                  return nullptr;
               auto i = this->info.presence.find_first_clear_from(this->info.startFrom);
               if (i < 0)
                  return nullptr;
               std::ptrdiff_t start = (std::ptrdiff_t) & this->buffer;
               std::ptrdiff_t addr = start + (element_size * i);
               this->info.on_allocate(i);
               return (void*)addr;
            }
            bool  try_free(void* mem) noexcept {
               auto block = this;
               #if _DEBUG
                  Block* previous = nullptr;
               #endif
               do {
                  std::ptrdiff_t m_addr  = (std::ptrdiff_t)mem;
                  std::ptrdiff_t b_start = (std::ptrdiff_t) & block->buffer;
                  std::ptrdiff_t b_end   = b_start + sizeof(block->buffer);
                  if (m_addr >= b_start && m_addr < b_end) {
                     m_addr -= b_start;
                     uint16_t index = m_addr / sizeof(mapped_type);
                     assert(m_addr % element_size == 0       && "Cannot free; element is not aligned.");
                     assert(block->info.presence.test(index) && "You're freeing something that was already free!");
                     block->info.on_free(index);
                     //
                     if (block != this && !block->has_any_slots_used()) {
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
                     return true;
                  }
                  #if _DEBUG
                     previous = block;
                  #endif
               } while (block = block->info.next);
               return false;
            }
            //
            Block* get_end() noexcept {
               auto block = this;
               while (block->info.next)
                  block = block->info.next;
               return block;
            }
            uint32_t count() const noexcept {
               uint32_t count = 1;
               auto block = this;
               while (block->info.next) {
                  block = block->info.next;
                  ++count;
               }
               return count;
            }
            //
            void prune() noexcept {
               for (auto block = this->info.next; block; block = block->info.next) {
                  if (!block->has_any_slots_used()) {
                     auto p = block->info.prev;
                     auto n = block->info.next;
                     if (p)
                        p->info.next = n;
                     if (n)
                        n->info.prev = p;
                     delete block;
                  }
               }
            }
         };
         struct Subheap {
            Block* first = new Block();
         };

         struct State {
            std::vector<Subheap*> subheaps;
            std::mutex subheapsLock;
            Block*     unowned     = nullptr;
            std::mutex unownedLock;
            //
            void register_subheap(Subheap* sub) noexcept {
               std::lock_guard<std::mutex> guard(this->subheapsLock);
               for (auto it = this->subheaps.begin(); it != this->subheaps.end(); ++it) {
                  Subheap* s = *it;
                  if (!s->first) {
                     *it = sub;
                     return;
                  }
               }
               this->subheaps.push_back(sub);
            }
            void take_over_subheap(Subheap* sub) noexcept {
               assert(sub->first && "This subheap was already taken over. How did it get here again?");
               auto block = sub->first;
               block->prune();
               if (!sub->first->has_any_slots_used()) {
                  block = block->info.next;
                  delete sub->first;
                  sub->first = nullptr;
                  if (!block)
                     return;
                  block->info.prev = nullptr;
               }
               std::lock_guard<std::mutex> guard(this->unownedLock);
               if (!this->unowned) {
                  this->unowned = block;
               } else {
                  auto appendTo = this->unowned->get_end();
                  appendTo->info.next = block;
                  block->info.prev = appendTo;
               }
               sub->first = nullptr;
            }
            void free(void* mem) noexcept {
               {
                  std::lock_guard<std::mutex> guard(this->subheapsLock);
                  for (auto it = this->subheaps.begin(); it != this->subheaps.end(); ++it) {
                     auto block = (*it)->first;
                     if (block && block->try_free(mem))
                        return;
                  }
               }
               {
                  std::lock_guard<std::mutex> guard(this->unownedLock);
                  if (this->unowned && this->unowned->try_free(mem))
                     return;
               }
               assert(false && "This heap cannot free memory that it isn't responsible for.");
            }
         };
         static State& _get_state() {
            static State instance;
            return instance;
         }

         struct SubheapHandle {
            Subheap* data;
            //
            SubheapHandle() {
               this->data = new Subheap;
               auto& state = multiheap::_get_state();
               state.register_subheap(this->data);
               //
               // Take the unowned block lists if possible:
               //
               {
                  std::lock_guard<std::mutex> guard(state.unownedLock);
                  if (state.unowned) {
                     delete this->data->first;
                     this->data->first = state.unowned;
                     state.unowned = nullptr;
                  }
               }
            }
            ~SubheapHandle() {
               if (this->data) {
                  multiheap::_get_state().take_over_subheap(this->data);
                  this->data = nullptr;
                  //
                  // Don't free (this->data); the heap state owns it.
               }
            }
            Subheap* get() const noexcept { return this->data; }
         };
         //
         inline thread_local static SubheapHandle current_thread;
         //
         static Subheap* _get_subheap() {
            return current_thread.get();
         }
         //
      public:
         static void* allocate() noexcept {
            auto t = multiheap::_get_subheap();
            assert(t && "Failed to get/create subheap?");
            assert(t->first && "The subheap has no block?");
            Block* block = t->first;
            Block* last = block;
            void* out = block->try_allocate();
            while (!out) {
               block = block->info.next;
               if (block)
                  last = block;
               else
                  break;
               out = block->try_allocate();
            }
            if (out)
               return out;
            if (!block) {
               assert(last && "Couldn't figure out how to create a new block.");
               auto next = new Block;
               last->info.next = next;
               next->info.prev = last;
               out = next->try_allocate();
               assert(out && "Allocation failed!");
            }
            return out;
         }
         static void free(void* mem) noexcept {
            multiheap::_get_state().free(mem);
         }
         //
         static void dump_stats() noexcept {
            auto& state = multiheap::_get_state();
            std::lock_guard<std::mutex> guard1(state.subheapsLock);
            std::lock_guard<std::mutex> guard2(state.unownedLock);
            printf("DUMPING INFORMATION FOR MULTI-HEAP...\n");
            printf("Number of extant sub-heaps: %d\n", state.subheaps.size());
            printf("Number of unowned blocks: %d\n", state.unowned ? state.unowned->count() : 0);
            if (state.subheaps.size()) {
               printf("STATS BY SUBHEAP:\n");
               for (uint32_t i = 0; i < state.subheaps.size(); i++) {
                  auto sh = state.subheaps[i];
                  printf("Subheap %d: ", i);
                  if (!sh->first) {
                     printf("<empty>\n");
                     continue;
                  }
                  printf("\n");
                  if (count_per_block < 150) {
                     printf("Overview by block:\n");
                     uint32_t blockCount = 0;
                     for (auto block = sh->first; block; block = block->info.next) {
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
               }
            }
            if (state.unowned && count_per_block < 150) {
               printf("UNOWNED BLOCKS:\n");
               printf("Overview by block:\n");
               uint32_t blockCount = 0;
               for (auto block = state.unowned; block; block = block->info.next) {
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
            printf("ALL INFORMATION DUMPED.\n");
         }
   };
   template<typename T, uint32_t count_per_block> class multiheap_allocator {
      //
      // An interface to multiheap that meets the requirements of Allocator.
      //
      public:
         using heap_type  = multiheap<T, count_per_block>;
         using value_type = T;
         template<typename U> struct rebind {
            using other = typename multiheap_allocator<U, count_per_block>;
         };
         //
         static heap_type& get_heap() noexcept {
            static heap_type instance;
            return instance;
         }
         multiheap_allocator() = default;
         template<class U> constexpr multiheap_allocator(const multiheap_allocator<U, count_per_block>&) noexcept {};
         //
         [[nodiscard]] T* allocate(std::size_t count) const {
            if (count > 1)
               throw std::invalid_argument("Can only allocate one.");
            return (T*) get_heap().allocate();
         }
         void deallocate(T* mem, std::size_t count) const noexcept {
            assert(count == 1 && "Can only free one.");
            get_heap().free(mem);
         }
         //
         // DISABLED; DO NOT USE
         // This allocator can only allocate one element at a time, which is what (max_size) is supposed to indicate. 
         // However, MSVC's internal std::_Tree class (used to power std::map and friends) misuses Allocator::max_size, 
         // comparing it to the tree's own size (i.e. treating it as the maximum number of elements that the allocator 
         // can *store* rather than the maximum that can be allocated at one time). Just another example of the STL 
         // Allocator interface being a nightmare.
         // std::size_t max_size() const noexcept { return 1; }
         //
         bool operator==(const multiheap_allocator& other) { return true; }
         bool operator!=(const multiheap_allocator& other) { return false; }
         template<typename U, uint32_t cb> bool operator==(const multiheap_allocator<U, cb>& other) { return false; }
         template<typename U, uint32_t cb> bool operator!=(const multiheap_allocator<U, cb>& other) { return true; }
   };

   namespace unit_tests {
      namespace multiheap {
         struct test_struct {
            uint32_t value = 0;
            //
            test_struct() {};
            test_struct(uint32_t a) : value(a) {};
            //
            static void* operator new(std::size_t sz);
            static void operator delete(void* ptr, std::size_t sz);
         };
         //
         void test();
      }
   }
}