#pragma once
#include <unordered_map>

namespace cobb {
   template<typename key_type, typename value_type> bool unordered_map_contains(const std::unordered_map<key_type, value_type>& map, const key_type& k) {
      //
      // Polyfill for C++20 (std::unordered_map::contains) intended for use in C++17. 
      // Better than wrapping (std::unordered_map::at) in a try/catch block, probably.
      //
      if (!map.bucket_count())
         return false;
      size_t bucket = map.bucket(k);
      for (auto it = map.begin(bucket); it != map.end(bucket); ++it) {
         if (it->first == k)
            return true;
      }
      return false;
   }
   template<typename key_type, typename value_type> const value_type& unordered_map_get_if_present(const std::unordered_map<key_type, value_type>& map, const key_type& k, value_type& fallback) {
      if (!map.bucket_count())
         return fallback;
      size_t bucket = map.bucket(k);
      for (auto it = map.begin(bucket); it != map.end(bucket); ++it) {
         if (it->first == k)
            return it->second;
      }
      return fallback;
   }
   template<typename key_type, typename value_type> value_type* unordered_map_get_if_present(const std::unordered_map<key_type, value_type*>& map, const key_type& k, value_type* fallback) {
      if (!map.bucket_count())
         return fallback;
      size_t bucket = map.bucket(k);
      for (auto it = map.begin(bucket); it != map.end(bucket); ++it) {
         if (it->first == k)
            return it->second;
      }
      return fallback;
   }
   template<typename key_type, typename value_type> value_type* unordered_map_get_if_present(const std::unordered_map<key_type, value_type*>& map, const key_type& k) {
      return unordered_map_get_if_present(map, k, (value_type*)nullptr);
   }
}
