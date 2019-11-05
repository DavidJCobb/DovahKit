#include "wavl_tree.h"
#include <iostream>

namespace cobb {
   namespace unit_tests {
      void wavl_tree() {
         cobb::wavl_tree<uint32_t, uint32_t> numbers;
         std::cout << "Testing wavl_tree...\n";
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
         numbers.remove(3);
         printf("Removed 3. Printing tree:\n");
         numbers.for_each([](uint32_t k, uint32_t v) {
            printf("Key: %d; value: %d\n", k, v);
            return false;
         });
         numbers.remove(11);
         printf("Removed 11. Printing tree:\n");
         numbers.for_each([](uint32_t k, uint32_t v) {
            printf("Key: %d; value: %d\n", k, v);
            return false;
         });
      }
   }
}