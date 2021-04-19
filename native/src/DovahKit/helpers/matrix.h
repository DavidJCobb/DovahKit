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
   template<int m, int n> class matrix {
      public:
         static constexpr int height = m;
         static constexpr int width  = n;
         static constexpr int count  = m * n;
         static constexpr bool is_square = m == n;
         //
         double data[height][width]; // yes, that's [m][n] == [y][x], because mathematicians think in terms of [row][column]

         static typename std::enable_if_t<is_square, matrix<m, n>> identity() noexcept {
            matrix<m, n> result;
            memset(&result.data, 0, sizeof(result.data));
            for (int i = 0; i < m; ++i)
               result.data[i][i] = 1.0F;
            return result;
         }
         
         inline const double& operator[](int i) const noexcept {
            return this->at(i);
         }
         double& operator[](int i) noexcept {
            return this->at(i);
         }
         const double& at(int i) const noexcept {
            int x = i % 3;
            int y = (i - x) / 3;
            return this->data[y][x];
         }
         double& at(int i) noexcept {
            int x = i % 3;
            int y = (i - x) / 3;
            return this->data[y][x];
         }

         void set_row(int i, double list[n]) noexcept {
            for (int j = 0; j < n; ++j)
               this->data[i][j] = list[j];
         }

         #pragma region multiply-by-matrix operator overloads
         // [ a , b , c ,     [ r , s , t ,     [ ar + bu + cx  ,  as + bv + cy  ,  at + bw + cz ,
         //   d , e , f ,  *    u , v , w ,  =    dr + eu + fx  ,  ds + ev + fy  ,  dt + ew + fz ,
         //   g , h , i ]       x , y , z ]       gr + hu + ix  ,  gs + hv + iy  ,  gt + hw + iz ]
         template<int o> matrix<m, o> operator*(const matrix<n, o>& other) { // multiply a (m,n) matrix by a (p,q) matrix; only possible if n == p
            matrix<m, o> result;
            for (int i = 0; i < m; ++i) {
               for (int j = 0; j < o; ++j) {
                  result.data[i][j] = 0.0F;
                  for (int k = 0; k < n; ++k) {
                     result.data[i][j] += this->data[i][k] * other.data[k][j];
                  }
               }
            }
            return result;
         }
         matrix<m, n>& operator*=(const matrix<n, n>& other) { // multiply-assign requires that we produce an (m,n) result; given n == p, we can only multiply-assign by an (n,n) matrix
            *this = (*this) * other;
            return *this;
         }
         #pragma endregion
         #pragma region multiply-by-scalar operator overloads
         matrix<m, n> operator*(double operand) const noexcept {
            matrix<m, n> result;
            for (int i = 0; i < m; ++i)
               for (int j = 0; j < n; ++j)
                  result.data[i][j] = this->data[i][j] * operand;
            return result;
         }
         matrix<m, n>& operator*=(double operand) noexcept {
            for (int i = 0; i < m; ++i)
               for (int j = 0; j < n; ++j)
                  this->data[i][j] *= operand;
            return *this;
         }
         #pragma endregion
         #pragma region multiply-by-column operator overload
         std::array<double, m> operator*(const std::array<double, n>& vec) const noexcept {
            //
            // This is conceptually equal to multiplying the (m,n) matrix by an (n,1) matrix, producing an (m,1) matrix.
            //
            std::array<double, m> result;
            for (int i = 0; i < m; ++i) {
               result[i] = 0.0F;
               for (int j = 0; j < n; ++j)
                  result[i] += this->data[i][j] * vec[j];
            }
            return result;
         }
         #pragma endregion

         inline typename std::enable_if_t<is_square, double> trace() const noexcept {
            double r = 0.0F;
            for (int i = 0; i < n; ++i)
               r += data[i][i];
            return r;
         }

         // [a, b, c,    [a, d, g,
         //  d, e, f, ->  b, e, h,
         //  g, h, i]     c, f, i]
         //
         // TIP: Transposing a rotation matrix is equivalent to swapping its handedness.
         //
         matrix<n, m> transpose() const noexcept {
            matrix<n, m> result;
            for (int v = 0; v < m; ++v)
               for (int u = 0; u < n; ++u)
                  result.data[u][v] = this->data[v][u];
            return result;
         }
         typename std::enable_if_t<is_square, void> transpose_in_place() noexcept {
            constexpr int cap = m > n ? m : n;
            for (int i = 0; i < cap; ++i)
               for (int j = 0; j < i; ++j)
                  std::swap(this->data[i][j], this->data[j][i]);
         }
   };
}