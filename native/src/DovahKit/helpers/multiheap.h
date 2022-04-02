#pragma once
#include <cassert>
#include <cstdint>
#include <mutex>
#include <thread>
#include <type_traits>
#include <vector>
#include "../helpers/bitset.h"

namespace cobb {
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
      // similarly to smart pointers and lock guards.) The effect of this is that alloc-
      // ations don't cause the entire heap to lock; locking is only necessary when adding 
      // or removing threads, or when freeing any element.
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
      // however, that methods like (force_free_all) are unusable when working with STL 
      // containers.)
      //
      // To that end, an interface is already available: multiheap_allocator, below.
      //
      public:
         using mapped_type = T;
         static constexpr size_t element_alignment = std::alignment_of_v<T>;
         static constexpr size_t element_size      = sizeof(T);
         static constexpr size_t stride            = element_size + (element_size % element_alignment);
         static constexpr size_t count_per_block   = count_per_block;
         
      protected:
         struct block_t;
         struct block_info {
            block_t* prev       = nullptr;
            block_t* next       = nullptr;
            uint32_t remaining  = count_per_block; // optimization for large block sizes
            uint32_t start_from = 0;               // optimization for large block sizes
            cobb::bitset<count_per_block> presence;
            //
            inline void on_allocate(uint32_t index) noexcept {
               this->presence.set(index);
               --this->remaining;
               this->start_from = index + 1;
            }
            inline void on_free(uint32_t index) noexcept {
               this->presence.reset(index);
               ++this->remaining;
               if (this->start_from > index)
                  this->start_from = index;
            }
         };
         struct block_t {
            block_info info;
            uint8_t    buffer[count_per_block * stride];
            //
            inline ~block_t() {
               auto* p = this->info.prev;
               auto* n = this->info.next;
               if (p)
                  p->info.next = n;
               if (n)
                  n->info.prev = p;
            }
            //
            inline bool has_free_slots()     const noexcept { return this->info.remaining != 0; };
            inline bool has_any_slots_used() const noexcept { return this->info.remaining < count_per_block; }
            void* try_allocate() { // function to be called on the head block only. allocates a single element
               if (!this->has_free_slots())
                  return nullptr;
               auto i = this->info.presence.find_first_clear_from(this->info.start_from);
               if (i < 0)
                  return nullptr;
               std::intptr_t start = (std::intptr_t) &this->buffer;
               std::intptr_t addr  = start + (stride * i);
               this->info.on_allocate(i);
               return (void*)addr;
            }
            bool try_free(void* mem) { // function to be called on the head block only. frees a single element (and if that leaves a non-head block empty, free that entire block)
               auto* block = this;
               do {
                  std::intptr_t m_addr  = (std::intptr_t)mem;
                  std::intptr_t b_start = (std::intptr_t) & block->buffer;
                  std::intptr_t b_end   = b_start + sizeof(block->buffer);
                  if (m_addr >= b_start && m_addr < b_end) {
                     m_addr -= b_start;
                     uint16_t index = m_addr / stride;
                     assert(m_addr % stride == 0             && "Cannot free; element is not aligned.");
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
               } while (block = block->info.next);
               return false;
            }
            
            inline block_t* get_end() noexcept {
               auto* block = this;
               while (block->info.next)
                  block = block->info.next;
               return block;
            }
            uint32_t count() const noexcept {
               uint32_t count = 1;
               auto* block = this;
               while (block->info.next) {
                  block = block->info.next;
                  ++count;
               }
               return count;
            }
            
            void prune() noexcept { // removes all empty blocks after this one
               auto* n = this->info.next;
               for (auto* block = n; block; block = n) {
                  n = block->info.next;
                  if (!block->has_any_slots_used())
                     delete block;
               }
            }
            
            #pragma region Helpers for State::force_destroy_all
            void destroy_elements() noexcept {
               auto& presence = this->info.presence;
               for (uint32_t i = 0; i < count_per_block; i++) {
                  if (presence.test(i)) {
                     std::intptr_t start = (std::intptr_t) & this->buffer;
                     std::intptr_t addr  = start + (stride * i);
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
         struct subheap {
            block_t* first = new block_t;
            #if _DEBUG
               std::thread::id threadID; // just so you can see the thread that owns this Subheap in a debugger
            #endif
         };

         class State {
            public:
               std::vector<subheap*> subheaps; // nullptr not allowed
               std::mutex subheaps_lock;
               block_t*   unowned = nullptr;
               std::mutex unowned_lock;
               
               void register_subheap(subheap& sub) noexcept {
                  std::lock_guard guard(this->subheaps_lock);
                  for (auto*& s : this->subheaps) {
                     if (!s->first) {
                        delete s;
                        s = &sub;
                        return;
                     }
                  }
                  this->subheaps.push_back(&sub);
               }
               void take_over_subheap(subheap& sub) noexcept {
                  assert(sub.first && "This subheap was already taken over. How did it get here again?");
                  std::lock_guard guard_1(this->subheaps_lock);
                  auto* block = sub.first;
                  block->prune();
                  if (!block->has_any_slots_used()) {
                     auto* n = block->info.next;
                     delete block;
                     sub.first = nullptr;
                     block = n;
                     if (!block)
                        return;
                  }
                  std::lock_guard guard_2(this->unowned_lock);
                  if (!this->unowned) {
                     this->unowned = block;
                  } else {
                     auto* append_to = this->unowned->get_end();
                     append_to->info.next = block;
                     block->info.prev = append_to;
                  }
                  sub.first = nullptr;
                  //
                  // We don't bother deleting (sub), but it's basically "dead" at this point. 
                  // When a new subheap comes along and registers itself, we'll iterate over 
                  // our subheap list, see the "dead" (sub) in our subheap vector, and delete 
                  // and replace it at that time.
                  //
                  // Something to look into, perhaps: should (subheap_handle) be made to take 
                  // ownership of "dead" subheaps and recycle them, instead of its current 
                  // behavior (always allocate a new subheap)?
               }
               void free(void* mem) noexcept {
                  {
                     std::lock_guard guard(this->subheaps_lock);
                     for (auto* s : this->subheaps) {
                        auto* block = s->first;
                        if (block && block->try_free(mem))
                           return;
                     }
                  }
                  {
                     std::lock_guard guard(this->unowned_lock);
                     if (this->unowned && this->unowned->try_free(mem))
                        return;
                  }
                  assert(false && "This heap cannot free memory that it isn't responsible for.");
               }
               
               void force_destroy_all() noexcept { // probably risky; i wouldn't recommend calling this ever
                  std::lock_guard<std::mutex> guard(this->subheaps_lock);
                  std::lock_guard<std::mutex> guard(this->unowned_lock);
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

               static State& get() {
                  static State instance;
                  return instance;
               }
         };
         static State& _get_state() {
            return State::get();
         }

         struct subheap_handle {
            subheap* data = nullptr;
            //
            subheap_handle() {}
            ~subheap_handle() {
               if (!this->data)
                  return;
               multiheap::_get_state().take_over_subheap(*this->data);
               this->data = nullptr; // Don't free (this->data); the heap state owns it.
            }

            void initialize() {
               if (this->data)
                  return;
               this->data = new subheap;
               #if _DEBUG
                  this->data->threadID = std::this_thread::get_id();
               #endif
               auto& state = multiheap::_get_state();
               state.register_subheap(*this->data);
               //
               // Take the unowned block lists if possible:
               //
               {
                  std::lock_guard guard(state.unowned_lock);
                  if (state.unowned) {
                     std::lock_guard guard(state.subheaps_lock); // we've already been registered, so this is necessary
                     assert(state.unowned != this->data->first);
                     delete this->data->first;
                     this->data->first = state.unowned;
                     state.unowned = nullptr;
                  }
               }
            }

            inline subheap* get() const noexcept { return this->data; }
         };
         //
         inline thread_local static subheap_handle current_thread;
         //
         static subheap* _get_subheap() {
            current_thread.initialize(); // closest we can get to lazy-initialization
            return current_thread.get();
         }
         
      public:
         static void* allocate() {
            auto t = multiheap::_get_subheap();
            assert(t        && "Failed to get/create subheap?");
            assert(t->first && "The subheap has no block?");
            block_t* block = t->first;
            block_t* last  = block;
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
               auto next = new block_t;
               last->info.next = next;
               next->info.prev = last;
               out = next->try_allocate();
               assert(out && "Allocation failed!");
            }
            return out;
         }
         static void free(void* mem) {
            multiheap::_get_state().free(mem);
         }
         //
         // Call destructors on every element in the heap, and then free them all. Obviously you should 
         // only use this when you're sure that nothing is using those elements anymore.
         static void force_destroy_all() {
            multiheap::_get_state().force_destroy_all();
         }
   };

   template<typename T, uint32_t count_per_block> class multiheap_allocator {
      //
      // An interface to multiheap that meets the Allocator named requirement.
      //
      public:
         using value_type = T;
      public:
         using heap_type  = multiheap<T, count_per_block>;
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
         // can *store* rather than the maximum that can be allocated in one call). As such, if we actually accurately 
         // indicate the maximum number of elements that we can allocate at one time, we will cause guaranteed crashes 
         // within Microsoft's Common Runtime DLL that are fiendishly difficult to debug.
         //
         // std::size_t max_size() const noexcept { return 1; }

         bool operator==(const multiheap_allocator& other) { return true; }
         bool operator!=(const multiheap_allocator& other) { return false; }
         template<typename U, uint32_t cb> bool operator==(const multiheap_allocator<U, cb>& other) { return false; }
         template<typename U, uint32_t cb> bool operator!=(const multiheap_allocator<U, cb>& other) { return true; }
   };
}