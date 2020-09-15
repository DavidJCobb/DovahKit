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
#include <cstdint>
#include <windows.h>
#include "intrusive_windows_defines.h"

namespace cobb {
   struct benchmark {
      LARGE_INTEGER start_time;
      LARGE_INTEGER end_time;
      LARGE_INTEGER elapsed_time;
      LARGE_INTEGER frequency;
      //
      inline void begin() {
         QueryPerformanceFrequency(&this->frequency);
         QueryPerformanceCounter(&this->start_time);
      }
      inline void end() {
         QueryPerformanceCounter(&this->end_time);
         this->elapsed_time.QuadPart  = this->end_time.QuadPart - this->start_time.QuadPart;
         this->elapsed_time.QuadPart *= 1000000;
         this->elapsed_time.QuadPart /= this->frequency.QuadPart;
      }
      inline uint32_t microseconds() const noexcept {
         return this->elapsed_time.QuadPart;
      }
      inline uint32_t milliseconds() const noexcept {
         return this->elapsed_time.QuadPart / 1000;
      }
   };
}