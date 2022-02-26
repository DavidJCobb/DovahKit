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

namespace cobb {
   template<size_t m, size_t n, typename T = double> class matrix { // row-major
      public:
         using value_type = T;
         //
         static constexpr size_t height    = m;
         static constexpr size_t width     = n;
         static constexpr size_t count     = m * n;
         static constexpr bool   is_square = m == n;

         static_assert(count > 0, "A cobb::matrix cannot be zero-size along either axis.");
         
         value_type data[height][width]; // yes, that's [m][n] == [y][x], because mathematicians think in terms of [row][column]

         static matrix<width, height> identity() noexcept requires (is_square) {
            matrix<width, height> result;
            memset(&result.data, 0, sizeof(result.data));
            for (int i = 0; i < width; ++i)
               result.data[i][i] = 1.0F;
            return result;
         }
         
         inline const value_type& operator[](size_t i) const noexcept {
            return this->at(i);
         }
         value_type& operator[](size_t i) noexcept {
            return this->at(i);
         }
         const value_type& at(size_t i) const noexcept {
            int x = i % width;
            int y = (i - x) / width;
            return this->data[y][x];
         }
         value_type& at(size_t i) noexcept {
            int x = i % width;
            int y = (i - x) / width;
            return this->data[y][x];
         }

         void set_row(int i, value_type list[width]) noexcept {
            for (int j = 0; j < width; ++j)
               this->data[i][j] = list[j];
         }

         #pragma region multiply-by-matrix operator overloads
         // [ a , b , c ,     [ r , s , t ,     [ ar + bu + cx  ,  as + bv + cy  ,  at + bw + cz ,
         //   d , e , f ,  *    u , v , w ,  =    dr + eu + fx  ,  ds + ev + fy  ,  dt + ew + fz ,
         //   g , h , i ]       x , y , z ]       gr + hu + ix  ,  gs + hv + iy  ,  gt + hw + iz ]
         template<int o> matrix<height, o> operator*(const matrix<width, o>& other) { // multiply a (m,n) matrix by a (p,q) matrix; only possible if n == p
            matrix<m, o> result;
            for (int i = 0; i < height; ++i) {
               for (int j = 0; j < o; ++j) {
                  result.data[i][j] = 0.0F;
                  for (int k = 0; k < width; ++k) {
                     result.data[i][j] += this->data[i][k] * other.data[k][j];
                  }
               }
            }
            return result;
         }
         matrix<height, width>& operator*=(const matrix<width, width>& other) { // multiply-assign requires that we produce an (m,n) result; given n == p, we can only multiply-assign by an (n,n) matrix
            *this = (*this) * other;
            return *this;
         }
         #pragma endregion
         #pragma region multiply-by-scalar operator overloads
         matrix<m, n> operator*(value_type operand) const noexcept {
            matrix<m, n> result;
            result *= operand;
            return result;
         }
         matrix<m, n>& operator*=(value_type operand) noexcept {
            for (int i = 0; i < height; ++i)
               for (int j = 0; j < width; ++j)
                  this->data[i][j] *= operand;
            return *this;
         }
         #pragma endregion
         #pragma region multiply-by-column operator overload
         std::array<value_type, m> operator*(const std::array<value_type, n>& vec) const noexcept {
            //
            // This is conceptually equal to multiplying the (m,n) matrix by an (n,1) matrix, producing an (m,1) matrix.
            //
            std::array<value_type, m> result;
            for (int i = 0; i < height; ++i) {
               result[i] = 0.0F;
               for (int j = 0; j < width; ++j)
                  result[i] += this->data[i][j] * vec[j];
            }
            return result;
         }
         #pragma endregion

         inline value_type trace() const noexcept requires (is_square) {
            value_type r = 0.0F;
            for (int i = 0; i < width; ++i)
               r += data[i][i];
            return r;
         }

         // [a, b, c,    [a, d, g,
         //  d, e, f, ->  b, e, h,
         //  g, h, i]     c, f, i]
         //
         // TIP: Transposing a rotation matrix is equivalent to swapping its handedness.
         //
         matrix<width, height> transpose() const noexcept {
            matrix<n, m> result;
            for (int v = 0; v < height; ++v)
               for (int u = 0; u < width; ++u)
                  result.data[u][v] = this->data[v][u];
            return result;
         }
         void transpose_in_place() noexcept requires (is_square) {
            constexpr int cap = height > width ? height : width;
            for (int i = 0; i < cap; ++i)
               for (int j = 0; j < i; ++j)
                  std::swap(this->data[i][j], this->data[j][i]);
         }
   };
}