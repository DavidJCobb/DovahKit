#include "wavl_tree.h"
#include "memory.h"
#include <iostream>

namespace {
   using _node_type = cobb::wavl_node<uint32_t, uint32_t>;
   struct _wavl_node_heap : public cobb::multithreaded_block_allocator<_node_type, 16000, 8> {
      public:
         inline static _wavl_node_heap& get() {
            static _wavl_node_heap instance;
            return instance;
         }
   };
   class _wavl_node_allocator : public std::allocator<_node_type> {
      public:
         _node_type* allocate(size_type n, const _node_type* hint = nullptr) {
            if (n > 1)
               throw std::invalid_argument("Cannot allocate more than 1.");
            return (_node_type*)_wavl_node_heap::get().allocate();
         }
         void deallocate(_node_type* p, size_type n) {
            _wavl_node_heap::get().free((void*)p);
         }
         //
         bool operator==(const _wavl_node_allocator& right) { return this == &right; }
         bool operator!=(const _wavl_node_allocator& right) { return this != &right; }
   };
}
namespace cobb {
   namespace unit_tests {
      void wavl_tree() {
         printf("Testing basic functionality...\n");
         {
            cobb::wavl_tree<uint32_t, uint32_t> numbers;
            numbers.set(1, 1);
            numbers.set(3, 3);
            numbers.set(11, 11);
            numbers.set(7, 7);
            numbers.set(5, 5);
            numbers.set(19, 19);
            numbers.for_each([](uint32_t k, uint32_t v) {
               printf("Key: %d; value: %d\n", k, v);
               return false;
            });
            {
               auto n = numbers.get(3);
               if (n)
                  printf("Key 3 holds value %d.\n", *n);
               else
                  printf("Key 3 reads as missing.\n");
            }
            {
               auto n = numbers.get(9999);
               if (n)
                  printf("Key 9999 holds value %d.\n", *n);
               else
                  printf("Key 9999 reads as missing.\n");
            }
            numbers.remove(3);
            printf("Removed 3. Printing tree:\n");
            numbers.for_each([](uint32_t k, uint32_t v) {
               printf("Key: %d; value: %d\n", k, v);
               return false;
            });
            //
            numbers.remove(11);
            printf("Removed 11. Printing tree:\n");
            numbers.for_each([](uint32_t k, uint32_t v) {
               printf("Key: %d; value: %d\n", k, v);
               return false;
            });
            //
            numbers.remove(1);
            printf("Removed 1. Printing tree:\n");
            numbers.remove(5);
            printf("Removed 5. Printing tree:\n");
            numbers.remove(7);
            printf("Removed 7. Printing tree:\n");
            numbers.for_each([](uint32_t k, uint32_t v) {
               printf("Key: %d; value: %d\n", k, v);
               return false;
            });
            //
            numbers.remove(9999);
            printf("Removed non-existent element 9999. Printing tree:\n");
            numbers.for_each([](uint32_t k, uint32_t v) {
               printf("Key: %d; value: %d\n", k, v);
               return false;
            });
         }
         printf("Testing STL methods...\n");
         {
            cobb::wavl_tree<uint32_t, uint32_t> numbers;
            numbers.set(1, 1);
            numbers.set(3, 3);
            numbers.set(11, 11);
            numbers.set(7, 7);
            numbers.set(5, 5);
            numbers.set(19, 19);
            //
            printf("Testing forward iterators:\n");
            for (auto it = numbers.begin(); it != numbers.end(); ++it) {
               printf("Key: %d; value: %d\n", it->first, it->second);
            }
            printf("Testing reverse iterators:\n");
            for (auto it = numbers.rbegin(); it != numbers.rend(); ++it) {
               printf("Key: %d; value: %d\n", it->first, it->second);
            }
         }
         printf("Testing allocator...\n");
         {
            auto registration = _wavl_node_heap::get().register_thread();
            cobb::wavl_tree<uint32_t, uint32_t, _wavl_node_allocator> numbers;
            numbers.set(1, 1);
            numbers.set(3, 3);
            numbers.set(11, 11);
            numbers.set(7, 7);
            numbers.set(5, 5);
            numbers.set(19, 19);
            //
            printf("Dumping heap info...\n");
            _wavl_node_heap::get().dumpStats();
            printf("Clearing...\n");
            numbers.clear();
            printf("Dumping heap info...\n");
            _wavl_node_heap::get().dumpStats();
         }
      }
   }
}