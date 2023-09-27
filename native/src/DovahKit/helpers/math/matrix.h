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
#include <array>
#include <optional>
#include <type_traits>

namespace cobb {
   namespace impl::matrix_intrinsics {
      extern bool is_diagonal_2x2(const float*);
      extern bool is_diagonal_3x3(const float*);
      extern void transpose_4x4(float*);
      extern void transpose_4x4(float* src, float* dst);
   }

   template<size_t Rows, size_t Columns, typename T = double> requires requires {
      requires Rows    > 0;
      requires Columns > 0;
      requires std::is_arithmetic_v<T>;
   } class matrix { // row-major
      public:
         static constexpr const size_t height    = Rows;
         static constexpr const size_t width     = Columns;
         static constexpr const size_t count     = height * width;
         static constexpr const bool   is_square = height == width;

         using value_type = T;
         using row_type   = std::array<T, width>;

         std::array<row_type, height> rows = {}; // yes, that's [m][n] == [y][x], matching math notation

         static constexpr const bool memory_is_contiguous = []() -> bool {
            if constexpr (Rows <= 1) {
               return true;
            } else {
               std::array<row_type, height> dummy_rows = {};
               return &dummy_rows[1][0] == &dummy_rows[0][Columns - 1] + 1;
            }
         }();

         static constexpr matrix<Rows, Columns, T> identity() noexcept requires (is_square);
         
         constexpr const row_type& operator[](size_t i) const noexcept;
         constexpr row_type& operator[](size_t i) noexcept {
            return const_cast<row_type&>(std::as_const(*this).operator[](i));
         }

         constexpr const value_type& at(size_t i) const noexcept;
         constexpr value_type& at(size_t i) noexcept {
            return const_cast<value_type&>(std::as_const(*this).at(i));
         }

         constexpr std::array<value_type, Rows> column(size_t which) const noexcept;

      public:
         constexpr value_type determinant() const noexcept requires (is_square && Rows == 2);
         constexpr value_type determinant() const noexcept requires (is_square && Rows == 3);
         constexpr value_type determinant() const noexcept requires (is_square && Rows > 3);

         constexpr std::optional<size_t> decompose_lup(matrix& lower, matrix& upper) const noexcept requires (is_square);
         constexpr std::optional<size_t> decompose_lup(matrix& lower, matrix& upper, matrix& permutation) const noexcept requires (is_square);

         constexpr bool is_diagonal() const noexcept requires (is_square); // true if all non-diagonal elements are zero
         constexpr bool is_lower_triangular() const noexcept requires (is_square); // true if all elements below/left  of the diagonal are zero
         constexpr bool is_upper_triangular() const noexcept requires (is_square); // true if all elements above/right of the diagonal are zero

         constexpr value_type trace() const noexcept requires (is_square);

         // TIP: Transposing a rotation matrix is equivalent to swapping its handedness.
         constexpr matrix transposed() const noexcept requires (is_square);
         constexpr void transpose() noexcept requires (is_square);

      public:
         #pragma region multiply-by-matrix operator overloads
         template<size_t o>
         constexpr matrix<Rows, o, T> operator*(const matrix<Columns, o, T>& other); // multiply a (m,n) matrix by a (p,q) matrix; only possible if n == p

         constexpr matrix<Rows, Columns, T>& operator*=(const matrix<Columns, Columns, T>& other); // multiply-assign requires that we produce an (m,n) result; given n == p, we can only multiply-assign by an (n,n) matrix
         #pragma endregion

         #pragma region multiply-by-scalar operator overloads
         template<typename OperandType>
         constexpr matrix operator*(OperandType operand) const noexcept requires std::is_arithmetic_v<OperandType>;

         template<typename OperandType>
         constexpr matrix& operator*=(OperandType operand) noexcept requires std::is_arithmetic_v<OperandType>;
         #pragma endregion

         #pragma region multiply-by-column operator overload
         constexpr std::array<value_type, Rows> operator*(const std::array<value_type, Columns>& vec) const noexcept;
         #pragma endregion
   };
}

#include "./matrix.inl"