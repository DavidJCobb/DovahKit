#include "multiheap.h"
#include <cstdlib>
#include <map>

namespace cobb_ex {
   namespace unit_tests {
      namespace multiheap {
         using test_struct_heap_type = cobb_ex::multiheap< test_struct, 20>;
         /*static*/ void* test_struct::operator new(std::size_t sz) {
            if (sz != sizeof(test_struct))
               return ::operator new(sz);
            return test_struct_heap_type::allocate();
         }
         /*static*/ void test_struct::operator delete(void* ptr, std::size_t sz) {
            if (sz != sizeof(test_struct))
               return ::operator delete(ptr, sz);
            return test_struct_heap_type::free(ptr);
         }
         //
         struct _total {
            std::mutex lock;
            std::vector<test_struct*> contents;
            //
            std::map<uint32_t, uint32_t, std::less<uint32_t>, cobb_ex::multiheap_allocator<std::pair<const uint32_t, uint32_t>, 20>> contentsMap;
         };
         void _test_struct_thread(_total* total) {
            printf("Thread %08X: Starting...", std::this_thread::get_id());
            std::vector<test_struct*> instances;
            for (uint32_t i = 0; i < 33; i++)
               instances.push_back(new test_struct);
            std::lock_guard<std::mutex> guard(total->lock);
            total->contents.reserve(total->contents.size() + instances.size());
            for (auto it = instances.begin(); it != instances.end(); ++it)
               total->contents.push_back(*it);
            printf("Thread %08X: Done.", std::this_thread::get_id());
            test_struct_heap_type::dump_stats();
         }
         void _test_map_thread(_total* total) {
            printf("Thread %08X: Starting...", std::this_thread::get_id());
            for (uint32_t i = 0; i < 33; i++) {
               std::lock_guard<std::mutex> guard(total->lock);
               auto index = rand();
               total->contentsMap[index] = i;
            }
            printf("Thread %08X: Done.", std::this_thread::get_id());
         }
         void test() {
             _total finalResults;
             std::thread threads[8];
             printf("Starting threads (struct test)...\n");
             for (uint32_t i = 0; i < 8; i++)
                threads[i] = std::thread(_test_struct_thread, &finalResults);
             for (uint32_t i = 0; i < 8; i++)
                threads[i].join();
             printf("All threads done.\n");
             test_struct_heap_type::dump_stats();
             for (uint32_t i = 0; i < finalResults.contents.size(); i++) {
                auto entry = finalResults.contents[i];
                printf("Entry %d: %p\n", i, entry);
                delete entry;
             }
             finalResults.contents.clear();
             printf("All contents cleared.\n");
             test_struct_heap_type::dump_stats();
             printf("\n");
             printf("Starting threads (map test)...\n");
             for (uint32_t i = 0; i < 8; i++)
                threads[i] = std::thread(_test_map_thread, &finalResults);
             for (uint32_t i = 0; i < 8; i++)
                threads[i].join();
             printf("All threads done.\n");
             test_struct_heap_type::dump_stats();
             for (auto it = finalResults.contentsMap.begin(); it != finalResults.contentsMap.end(); ++it) {
                printf("Entry %u: %u\n", it->first, it->second);
             }
             finalResults.contentsMap.clear();
             printf("All map contents cleared.\n");
         }
      }
   }
}