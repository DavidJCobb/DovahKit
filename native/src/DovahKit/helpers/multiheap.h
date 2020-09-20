#pragma once
#include <cassert>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>
#include "../helpers/bitset.h"

namespace cobb {
   extern void _multiheap_dumper(void* s, uint32_t count_per_block); // defined this way just so that the code to dump heap information for debugging isn't just sitting out in the header
   //
   template<typename T, uint32_t count_per_block> class multiheap {
      //
      // MULTIHEAP
      //
      // This is a multi-threaded block heap, allocating instances of a class in batches 
      // and then parcelling out the memory, instead of letting them be malloc'd one at 
      // a time.
      //
      // Internally, this heap is split into multiple sub-heaps. When a thread attempts 
      // to allocate memory, a new sub-heap is created for it; when the thread dies, the 
      // sub-heap is automatically destroyed and its blocks are moved to a list of unowned 
      // blocks. (This is accomplished using a thread_local struct that leverages RAII, 
      // like a smart pointer or lock guard.) The effect of this is that allocations don't 
      // cause the entire heap to lock; locking is only necessary when adding or removing 
      // threads, or when freeing any element.
      //
      // To help prevent memory fragmentation, if a sub-heap is created while there are 
      // unowned blocks, those blocks are assigned to the new sub-heap.
      //
      // The thread management happens entirely within this class, which means that it 
      // is safe to use with AllocatorAwareContainer classes like std::map. Those classes 
      // will use std::allocator::rebind to effectively discard the allocator you actually 
      // give them and create a new one (inaccessible to the outside) templated on their 
      // internal node types; however, because you never actually need to access this 
      // heap from the outside (i.e. you don't need your threads to manually register 
      // with it or any such), this isn't a problem. (Allocator rebinding does mean, 
      // however, that methods like (force_free_all) and (dump_stats) are unusable when 
      // working with STL containers.)
      //
      // To that end, an interface is already available: multiheap_allocator, below.
      //
      friend void _multiheap_dumper(void* s, uint32_t count_per_block);
      public:
         using mapped_type = T;
         static constexpr uint32_t element_size    = sizeof(T);
         static constexpr uint32_t count_per_block = count_per_block;
         //
      protected:
         struct Block;
         struct BlockInfo {
            Block*   prev      = nullptr;
            Block*   next      = nullptr;
            uint32_t remaining = count_per_block; // optimization for large block sizes
            uint32_t startFrom = 0; // optimization for large block sizes
            cobb::bitset<count_per_block> presence;
            //
            inline void on_allocate(uint32_t index) noexcept {
               this->presence.set(index);
               --this->remaining;
               this->startFrom = index;
            }
            inline void on_free(uint32_t index) noexcept {
               this->presence.reset(index);
               ++this->remaining;
               if (this->startFrom > index)
                  this->startFrom = index;
            }
         };
         struct Block {
            BlockInfo info;
            uint8_t   buffer[count_per_block * element_size];
            //
            ~Block() {
               auto p = this->info.prev;
               auto n = this->info.next;
               if (p)
                  p->info.next = n;
               if (n)
                  n->info.prev = p;
            }
            //
            inline bool has_free_slots()     const noexcept { return this->info.remaining; };
            inline bool has_any_slots_used() const noexcept { return this->info.remaining < count_per_block; }
            void* try_allocate() noexcept { // function to be called on the head block only. allocates a single element
               if (!this->has_free_slots())
                  return nullptr;
               auto i = this->info.presence.find_first_clear_from(this->info.startFrom);
               if (i < 0)
                  return nullptr;
               std::ptrdiff_t start = (std::ptrdiff_t) & this->buffer;
               std::ptrdiff_t addr  = start + (element_size * i);
               this->info.on_allocate(i);
               return (void*)addr;
            }
            bool  try_free(void* mem) noexcept { // function to be called on the head block only. frees a single element (and if that leaves a non-head block empty, free that entire block)
               auto block = this;
               #if _DEBUG
                  Block* previous = nullptr; // useless variable; when debugging, allows us to better understand where we were if something goes wrong and (block) goes bad
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
                        // The block that contained (mem) was not a list head, and is now empty. 
                        // Delete it.
                        //
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
            void prune() noexcept { // removes all empty blocks after this one
               auto n = this->info.next;
               for (auto block = n; block; block = n) {
                  n = block->info.next;
                  if (!block->has_any_slots_used())
                     delete block;
               }
            }
            //
            #pragma region Helpers for State::force_destroy_all
            void destroy_elements() noexcept {
               auto& presence = this->info.presence;
               for (uint32_t i = 0; i < count_per_block; i++) {
                  if (presence.test(i)) {
                     std::ptrdiff_t start = (std::ptrdiff_t) & this->buffer;
                     std::ptrdiff_t addr  = start + (element_size * i);
                     //
                     auto element = (mapped_type*)addr;
                     element->~mapped_type();
                  }
               }
               presence.clear();
               this->info.remaining = count_per_block;
               //
               memset(this->buffer, 0, sizeof(this->buffer));
            }
            void destroy_and_prune_list() noexcept {
               auto last = this;
               while (last->info.next)
                  last = last->info.next;
               auto prev = last->info.prev;
               do {
                  last->destroy_elements();
                  if (last != this) // never delete the first block in a list
                     delete last;
                  //
                  last = prev;
                  if (prev)
                     prev = prev->info.prev;
               } while (last);
            }
            #pragma endregion
         };
         struct Subheap {
            Block* first = new Block;
            #if _DEBUG
               std::thread::id threadID; // just so you can see the thread that owns this Subheap in a debugger
            #endif
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
                  auto n = block->info.next;
                  delete block;
                  sub->first = nullptr;
                  block = n;
                  if (!block)
                     return;
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
            //
            void force_destroy_all() noexcept { // probably risky; i wouldn't recommend calling this ever
               std::lock_guard<std::mutex> guard(this->subheapsLock);
               std::lock_guard<std::mutex> guard(this->unownedLock);
               for (auto it = this->subheaps.begin(); it != this->subheaps.end(); ++it) {
                  auto last = (*it)->first;
                  if (last) {
                     last->destroy_and_prune_list();
                     //
                     // Note: Don't destroy the subheap's first block; otherwise crashes will occur, as you delete the main thread's 
                     // subheap out from under it.
                  }
               }
               if (this->unowned) {
                  this->unowned->destroy_and_prune_list();
                  delete this->unowned;
                  this->unowned = nullptr;
               }
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
               #if _DEBUG
                  this->data->threadID = std::this_thread::get_id();
               #endif
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
                  #if _DEBUG
                     this->data->threadID = std::thread::id();
                  #endif
                  multiheap::_get_state().take_over_subheap(this->data);
                  this->data = nullptr; // Don't free (this->data); the heap state owns it.
               }
            }
            inline Subheap* get() const noexcept { return this->data; }
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
            assert(t        && "Failed to get/create subheap?");
            assert(t->first && "The subheap has no block?");
            Block* block = t->first;
            Block* last  = block;
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
         // Call destructors on every element in the heap, and then free them all. Obviously you should 
         // only use this when you're sure that nothing is using those elements anymore.
         static void force_destroy_all() noexcept {
            multiheap::_get_state().force_destroy_all();
         }
         //
         static void dump_stats() noexcept {
            auto& state = multiheap::_get_state();
            _multiheap_dumper(&state, count_per_block);
         }
   };
   template<typename T, uint32_t count_per_block> class multiheap_allocator {
      //
      // An interface to multiheap that meets the Allocator named requirement.
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

         // COMMENTED OUT; DISABLED; DO NOT USE
         // This allocator can only allocate one element at a time, which is what (max_size) is supposed to indicate. 
         // However, MSVC's internal std::_Tree class (used to power std::map and friends) misuses Allocator::max_size, 
         // comparing it to the tree's own size (i.e. treating it as the maximum number of elements that the allocator 
         // can *store* rather than the maximum that can be allocated at one time). As such, if we actually accurately 
         // indicate the maximum number of elements that we can allocate at one time, we will cause guaranteed crashes 
         // within Microsoft's Common Runtime DLL that are fiendishly difficult to debug.
         //
         // std::size_t max_size() const noexcept { return 1; }

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