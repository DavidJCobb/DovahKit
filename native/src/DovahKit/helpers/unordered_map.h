/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
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
