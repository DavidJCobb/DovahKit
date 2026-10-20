#pragma once
#include <cassert>
#include <cstdint>
#include <limits>
#include <mutex>
#if _DEBUG
   #include <thread> // so subheaps can know their owning threads' IDs for debugging purposes
#endif
#include <type_traits>
#include <vector>
#include "../containers/bitset.h"

namespace lu::memory {
   template<typename T, size_t CountPerSlab>
   class threaded_slab_heap {
      public:
         using mapped_type = T;
         static constexpr const size_t element_alignment = std::alignment_of_v<T>;
         static constexpr const size_t element_size      = sizeof(T);
         static constexpr const size_t stride            = element_size + (element_alignment - (element_size % element_alignment));
         static constexpr const size_t count_per_slab    = CountPerSlab;

         static_assert(count_per_slab > 0, "You cannot define a multiheap with zero capacity per slab.");
         
      protected:
         // This is the smallest unsigned integer type capable of representing the 
         // value `count_per_slab`.
         using element_index_type = std::conditional_t<
            (count_per_slab <= (std::numeric_limits<uint8_t>::max)()),
            uint8_t,
            std::conditional_t<
               (count_per_slab <= (std::numeric_limits<uint16_t>::max)()),
               uint16_t,
               std::conditional_t<
                  (count_per_slab <= (std::numeric_limits<uint32_t>::max)()),
                  uint32_t,
                  size_t
               >
            >
         >;

         struct slab_type;

         struct slab_info {
            slab_type* prev = nullptr;
            slab_type* next = nullptr;
            element_index_type remaining  = count_per_slab; // optimization for large block sizes
            element_index_type start_from = 0;               // optimization for large block sizes
            bitset<count_per_slab> presence;
            #if _DEBUG
               std::thread::id original_thread_id; // just so you can see the thread that spawned this block in a debugger
            #endif
            
            constexpr void on_allocate(element_index_type index) noexcept;
            constexpr void on_free(element_index_type index) noexcept;
         };

         struct slab_type {
            slab_info info;
            alignas(element_alignment) uint8_t buffer[count_per_slab * stride];
            
            constexpr ~slab_type() {
               auto* p = this->info.prev;
               auto* n = this->info.next;
               if (p)
                  p->info.next = n;
               if (n)
                  n->info.prev = p;
            }
            
            constexpr bool has_free_slots()     const noexcept { return this->info.remaining != 0; };
            constexpr bool has_any_slots_used() const noexcept { return this->info.remaining < count_per_slab; }

            // Allocates and returns a single element, if any slots remain available in this slab. 
            // Returns nullptr otherwise.
            void* try_allocate();

            // Check if `mem` is inside of this slab's buffer (and is properly aligned). If so, 
            // then destroy the element at that address, mark the slot as clear, and return true. 
            // If the element isn't found, return false.
            bool try_free(void* mem);
            
            constexpr slab_type* get_end() noexcept;
            
            // Remove all empty slabs after this slab.
            void prune() noexcept;
            
            #pragma region Helpers for State::force_destroy_all
               void destroy_all_elements() noexcept;
               void destroy_and_prune_list() noexcept;
            #pragma endregion
         };

         struct subheap {
            slab_type* first = new slab_type; // nullptr not allowed except during/after a subheap is "killed"
            std::mutex alloc_free_lock;
            #if _DEBUG
               std::thread::id thread_id; // just so you can see the thread that owns this subheap in a debugger
            #endif

            void* allocate();
            bool try_free(void* mem);
         };

         class State {
            public:
               std::mutex lock;

               std::vector<subheap*> subheaps; // nullptr not allowed
               slab_type* unowned_slabs = nullptr;
               
            public:
               // This should be called on a newly-created subheap to allow it to be centrally 
               // managed (see caller `subheap_handle::initialize`).
               void register_new_subheap(subheap& sub) noexcept;

               // This should be called on a subheap to destroy it, when the subheap's owning 
               // thread is closing.
               void kill_subheap(subheap& sub) noexcept;

               // Free an instance of the mapped type allocated via this multiheap.
               void free(void* mem) noexcept;
               
               // Forcibly free all instances of the mapped type allocated via this multiheap. 
               // Probably risky; I wouldn't recommend calling this.
               void force_destroy_all() noexcept;

            public:
               static State& get() {
                  static State instance;
                  return instance;
               }
         };
         static State& _get_state() {
            return State::get();
         }

         struct subheap_handle {
            subheap_handle();
            ~subheap_handle();

            subheap* data = nullptr;

            void initialize();

            constexpr subheap* get() const noexcept { return this->data; }
         };
         
         inline thread_local static subheap_handle current_thread;
         
         static subheap* _get_subheap() {
            current_thread.initialize(); // closest we can get to lazy-initialization
            return current_thread.get();
         }
         
      public:
         static void* allocate() {
            auto* t = threaded_slab_heap::_get_subheap();
            assert(t && "Failed to get/create subheap?");
            return t->allocate();
         }
         static void free(void* mem) {
            threaded_slab_heap::_get_state().free(mem);
         }
         
         // Call destructors on every element in the heap, and then free them all. Obviously you should 
         // only use this when you're sure that nothing is using those elements anymore.
         static void force_destroy_all() {
            threaded_slab_heap::_get_state().force_destroy_all();
         }
   };
}

#include "./threaded_slab_heap.inl"