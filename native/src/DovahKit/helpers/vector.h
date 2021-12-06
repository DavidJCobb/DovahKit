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
#include <vector>

namespace cobb {
   //
   // Move a range of elements within a vector. Assumes that the range is valid.
   // Per: <https://stackoverflow.com/a/7533658>
   //
   template <typename T> void move_range(std::vector<T>& v, size_t start, size_t length, size_t to) {
      typename std::vector<T>::iterator first, middle, last;
      if (start == to || !length)
         return;
      assert(start + length <= v.size() && "The range to be moved extends past the end of the vector.");
      if (start < to) {
         first  = v.begin() + start;
         middle = first + length;
         last   = v.begin() + to + length;
      } else {
         first  = v.begin() + to;
         middle = v.begin() + start;
         last   = middle + length;
      }
      std::rotate(first, middle, last);
   }

   //
   // Quickly erase from an std::vector by avoiding having to shuffle all elements 
   // when erasing values from the middle: we just move the to-be-erased value to 
   // the end and then erase it.
   //
   // Finds the first matching value and erases it.
   //
   template<typename T> void unordered_erase(std::vector<T>& v, const T& value) {
      auto it = std::find(v.begin(), v.end(), value);
      if (it != v.end()) {
         std::iter_swap(it, v.end() - 1);
         v.erase(v.end() - 1);
      }
   }
   template<typename T, class unary_predicate_t> void unordered_erase(std::vector<T>& v, unary_predicate_t functor) {
      auto it = std::find_if(v.begin(), v.end(), functor);
      if (it != v.end()) {
         std::iter_swap(it, v.end() - 1);
         v.erase(v.end() - 1);
      }
   }
}