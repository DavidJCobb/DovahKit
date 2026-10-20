#pragma once
#include <cassert>
#include <stdexcept>
#include "./threaded_slab_heap.h"

namespace lu::memory {
   template<typename T, uint32_t count_per_slab>
   class threaded_slab_allocator {
      public:
         using difference_type = std::ptrdiff_t;
         using size_type       = std::size_t;
         using value_type      = T;

         using heap_type  = threaded_slab_heap<value_type, count_per_slab>;

         template<typename U>
         struct rebind {
            using other = typename threaded_slab_allocator<U, count_per_slab>;
         };
         
         threaded_slab_allocator() = default;
         template<class U> constexpr threaded_slab_allocator(const threaded_slab_allocator<U, count_per_slab>&) noexcept {};
         
         [[nodiscard]] value_type* allocate(std::size_t count) const {
            if (count > 1)
               throw std::invalid_argument("Can only allocate one.");
            return (T*) heap_type::allocate();
         }
         void deallocate(value_type* mem, std::size_t count) const noexcept {
            assert(count == 1 && "Can only free one.");
            heap_type::free(mem);
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

         bool operator==(const threaded_slab_allocator& other) { return true; }
         bool operator!=(const threaded_slab_allocator& other) { return false; }
         template<typename U, uint32_t cb> bool operator==(const threaded_slab_allocator<U, cb>& other) { return false; }
         template<typename U, uint32_t cb> bool operator!=(const threaded_slab_allocator<U, cb>& other) { return true; }
   };
}
