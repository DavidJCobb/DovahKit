#pragma once

class FormStub;

//
// Block-allocating elements in a std::map is significantly slower than using a std::unordered_map 
// with no custom allocator. Still, I'm retaining this in case it comes in handy in the future.
//
#define COBB_ESP_BLOCK_ALLOCATE_MAP_PAIRS 0

#if COBB_ESP_BLOCK_ALLOCATE_MAP_PAIRS == 1
   #include "helpers/multiheap.h"

   using FormMapPair = std::pair<const uint32_t, FormStub*>;
   using FormMapAllocator = cobb::multiheap_allocator<FormMapPair, 16000>;
   typedef std::map<uint32_t, FormStub*, std::less<uint32_t>, FormMapAllocator> map_of_forms;
#else
   typedef std::unordered_map<uint32_t, FormStub*> map_of_forms;
#endif