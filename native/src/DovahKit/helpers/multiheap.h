#pragma once
#include <cassert>
#include <cstdint>
#include <limits>
#include <mutex>
#include <stdexcept> // for cobb::multiheap_allocator
#if _DEBUG
   #include <thread> // so subheaps can know their owning threads' IDs for debugging purposes
#endif
#include <type_traits>
#include <vector>
#include "../helpers/bitset.h"

namespace cobb {
   template<typename T, size_t CountPerBlock>
   class multiheap {
      public:
         using mapped_type = T;
         static constexpr const size_t element_alignment = std::alignment_of_v<T>;
         static constexpr const size_t element_size      = sizeof(T);
         static constexpr const size_t stride            = element_size + ((element_alignment - (element_size % element_alignment)) % element_alignment);
         static constexpr const size_t count_per_block   = CountPerBlock;

         static_assert(count_per_block > 0, "You cannot define a multiheap with zero capacity per block.");
         
      protected:
         // This is the smallest unsigned integer type capable of representing the 
         // value `count_per_block`.
         using element_index_type = std::conditional_t<
            (count_per_block <= (std::numeric_limits<uint8_t>::max)()),
            uint8_t,
            std::conditional_t<
               (count_per_block <= (std::numeric_limits<uint16_t>::max)()),
               uint16_t,
               std::conditional_t<
                  (count_per_block <= (std::numeric_limits<uint32_t>::max)()),
                  uint32_t,
                  size_t
               >
            >
         >;

         struct block_t;

         struct block_info {
            block_t* prev = nullptr;
            block_t* next = nullptr;
            element_index_type remaining  = count_per_block; // optimization for large block sizes
            element_index_type start_from = 0;               // optimization for large block sizes
            cobb::bitset<count_per_block> presence;
            #if _DEBUG
               std::thread::id original_thread_id; // just so you can see the thread that spawned this block in a debugger
            #endif
            
            constexpr void on_allocate(element_index_type index) noexcept;
            constexpr void on_free(element_index_type index) noexcept;
         };

         struct block_t {
            block_info info;
            uint8_t    buffer[count_per_block * stride];
            
            constexpr ~block_t() {
               auto* p = this->info.prev;
               auto* n = this->info.next;
               if (p)
                  p->info.next = n;
               if (n)
                  n->info.prev = p;
            }
            
            constexpr bool has_free_slots()     const noexcept { return this->info.remaining != 0; };
            constexpr bool has_any_slots_used() const noexcept { return this->info.remaining < count_per_block; }

            // Allocates and returns a single element, if any slots remain available in this block. 
            // Returns nullptr otherwise.
            void* try_allocate();

            // Check if `mem` is inside of this block's buffer (and is properly aligned). If so, 
            // then destroy the element at that address, mark the slot as clear, and return true. 
            // If the element isn't found, return false.
            bool try_free(void* mem);
            
            constexpr block_t* get_end() noexcept;
            
            // Remove all empty blocks after this block.
            void prune() noexcept;
            
            #pragma region Helpers for State::force_destroy_all
               void destroy_all_elements() noexcept;
               void destroy_and_prune_list() noexcept;
            #pragma endregion
         };

         struct subheap {
            block_t*   first = new block_t; // nullptr not allowed except during/after a subheap is "killed"
            std::mutex alloc_free_lock;
            #if _DEBUG
               std::thread::id thread_id; // just so you can see the thread that owns this Subheap in a debugger
            #endif

            void* allocate();
            bool try_free(void* mem);
         };

         class State {
            public:
               std::mutex lock;

               std::vector<subheap*> subheaps; // nullptr not allowed
               block_t* unowned_blocks = nullptr;
               
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
            auto* t = multiheap::_get_subheap();
            assert(t && "Failed to get/create subheap?");
            return t->allocate();
         }
         static void free(void* mem) {
            multiheap::_get_state().free(mem);
         }
         
         // Call destructors on every element in the heap, and then free them all. Obviously you should 
         // only use this when you're sure that nothing is using those elements anymore.
         static void force_destroy_all() {
            multiheap::_get_state().force_destroy_all();
         }
   };

   template<typename T, uint32_t count_per_block>
   class multiheap_allocator {
      public:
         using value_type = T;
         using heap_type  = multiheap<value_type, count_per_block>;

         template<typename U>
         struct rebind {
            using other = typename multiheap_allocator<U, count_per_block>;
         };
         
         static heap_type& get_heap() noexcept {
            static heap_type instance;
            return instance;
         }
         multiheap_allocator() = default;
         template<class U> constexpr multiheap_allocator(const multiheap_allocator<U, count_per_block>&) noexcept {};
         
         [[nodiscard]] value_type* allocate(std::size_t count) const {
            if (count > 1)
               throw std::invalid_argument("Can only allocate one.");
            return (T*) get_heap().allocate();
         }
         void deallocate(value_type* mem, std::size_t count) const noexcept {
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

#include "./multiheap.inl"