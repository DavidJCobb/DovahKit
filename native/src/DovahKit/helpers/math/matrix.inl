#pragma once
#include "./matrix.h"

#include "./abs.h"

#pragma push_macro("CLASS_NAME")
#pragma push_macro("TEMPLATE_PARAMS")
#define TEMPLATE_PARAMS template<size_t Rows, size_t Columns, typename T> requires requires { requires Rows > 0; requires Columns > 0; requires std::is_arithmetic_v<T>; }
#define CLASS_NAME matrix<Rows, Columns, T>

namespace cobb {
   TEMPLATE_PARAMS
   /*static*/ constexpr CLASS_NAME CLASS_NAME::identity() noexcept requires (is_square) {
      CLASS_NAME result;
      for (int i = 0; i < width; ++i)
         result.rows[i][i] = 1.0F;
      return result;
   }

   TEMPLATE_PARAMS
   constexpr const CLASS_NAME::row_type& CLASS_NAME::operator[](size_t i) const noexcept {
      return this->rows[i];
   }

   TEMPLATE_PARAMS
   constexpr const CLASS_NAME::value_type& CLASS_NAME::at(size_t i) const noexcept {
      int x = i % width;
      int y = (i - x) / width;
      return this->rows[y][x];
   }

   TEMPLATE_PARAMS
   constexpr std::array<typename CLASS_NAME::value_type, Rows> CLASS_NAME::column(size_t which) const noexcept {
      if constexpr (Rows == 3) {
         return {
            this->rows[0][which],
            this->rows[1][which],
            this->rows[2][which]
         };
      }
      std::array<value_type, Rows> out = {};
      for (size_t i = 0; i < Rows; ++i)
         out[i] = this->rows[i][which];
      return out;
   }

   TEMPLATE_PARAMS
   constexpr typename CLASS_NAME::value_type CLASS_NAME::determinant() const noexcept requires (is_square && Rows == 2) {
      return at(0) * at(3) - at(1) * at(2); // ad - bc
   }

   TEMPLATE_PARAMS
   constexpr typename CLASS_NAME::value_type CLASS_NAME::determinant() const noexcept requires (is_square && Rows == 3) {
      // [a, b, c,
      //  d, e, f,
      //  g, h, i]
      //
      // ergo: aei + bfg + cdh - ceg - bdi - afh
      return at(0)*at(4)*at(8) +
             at(1)*at(5)*at(6) +
             at(2)*at(3)*at(7)
             -
             at(2)*at(4)*at(6) -
             at(1)*at(3)*at(8) -
             at(0)*at(5)*at(7);
   }

   TEMPLATE_PARAMS
   constexpr typename CLASS_NAME::value_type CLASS_NAME::determinant() const noexcept requires (is_square && Rows > 3) {
      matrix lower;
      matrix upper;
      auto   swaps = this->decompose_lup(lower, upper);
      if (!swaps.has_value())
         return 0;
         
      value_type out = 1;
      for (size_t i = 0; i < width; ++i) {
         out *= upper[i][i];
      }
      return out * (swaps.value() % 2) ? -1 : 1;
   }


   TEMPLATE_PARAMS
   constexpr std::optional<size_t> CLASS_NAME::decompose_lup(matrix& lower, matrix& upper) const noexcept requires (is_square) {
      constexpr const size_t dimension = width;
   
      upper = *this;
      lower = matrix::identity();
   
      size_t swaps = 0;
   
      for (size_t i = 0; i < dimension; ++i) {
      
         // If this row's diagonal is zero, then permutate the row until it isn't 
         // or until we fail.
         if (cobb::abs(upper[i][i]) < 0.0001) {
            size_t k = i + 1;
            for (; k < dimension; ++k)
               if (upper[k][i] >= 0.0001)
                  break;
            //
            if (k >= dimension) // Didn't find a row to swap with.
               return {};
            //
            std::swap(upper[i], upper[k]);
            ++swaps;
         }
      
         // Now we continue onward with the actual factorization:
         for (size_t j = i + 1; j < dimension; ++j) {
            lower[j][i] = upper[j][i] / upper[i][i];
         
            // per-row operations:
            for (size_t x = 0; x < dimension; ++x)
               upper[j][x] -= lower[j][i] * upper[i][x];
         }
      }

      return swaps;
   }

   //
   // Based on this tremendously helpful source:
   // <https://johnfoster.pge.utexas.edu/numerical-methods-book/LinearAlgebra_LU.html#Psuedocode-for-a-simple-$\mathbf{PLU}$-factorization>
   // <https://archive.ph/uhyJm>
   //
   TEMPLATE_PARAMS
   constexpr std::optional<size_t> CLASS_NAME::decompose_lup(matrix& lower, matrix& upper, matrix& permutation) const noexcept requires (is_square) {
      constexpr const size_t dimension = width;

      upper       = *this;
      lower       = matrix::identity();
      permutation = matrix::identity();

      size_t swaps = 0;

      for (size_t i = 0; i < dimension; ++i) {

         // If this row's diagonal is zero, then permutate the row until it isn't 
         // or until we fail.
         if (cobb::abs(upper[i][i]) < 0.0001) {
            size_t k = i + 1;
            for (; k < dimension; ++k)
               if (upper[k][i] >= 0.0001)
                  break;
            //
            if (k >= dimension) // Didn't find a row to swap with.
               return {};
            //
            std::swap(upper[i],       upper[k]);
            std::swap(permutation[i], permutation[k]);
            ++swaps;
         }

         // Now we continue onward with the actual factorization:
         for (size_t j = i + 1; j < dimension; ++j) {
            lower[j][i] = upper[j][i] / upper[i][i];

            // per-row operations:
            for (size_t x = 0; x < dimension; ++x)
               upper[j][x] -= lower[j][i] * upper[i][x];
         }
      }

      return swaps;
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::is_diagonal() const noexcept requires (is_square) {
      if (!std::is_constant_evaluated()) {
         if constexpr (memory_is_contiguous && std::is_same_v<value_type, float>) {
            if constexpr (Rows == 2) {
               return impl::matrix_intrinsics::is_diagonal_2x2(&rows);
            } else if constexpr (Rows == 3) {
               return impl::matrix_intrinsics::is_diagonal_3x3(&rows);
            }
         }
      }
      for (size_t y = 0; y < Rows; ++y) {
         for (size_t x = 0; x < Columns; ++x) {
            if (x == y)
               continue;
            if (cobb::abs(rows[y][x]) > 0.0001)
               return false;
         }
      }
      return true;
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::is_lower_triangular() const noexcept requires (is_square) {
      for (size_t y = 1; y < Rows; ++y) {
         for (size_t x = 0; x < y; ++x) {
            if (cobb::abs(rows[y][x]) > 0.0001)
               return false;
         }
      }
      return true;
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::is_upper_triangular() const noexcept requires (is_square) {
      for (size_t y = 0; y < Rows - 1; ++y) {
         for (size_t x = y + 1; x < Columns; ++x) {
            if (cobb::abs(rows[y][x]) > 0.0001)
               return false;
         }
      }
      return true;
   }

   TEMPLATE_PARAMS
   constexpr typename CLASS_NAME::value_type CLASS_NAME::trace() const noexcept requires (is_square) {
      if constexpr (Rows == 2) {
         return rows[0][0] + rows[1][1];
      } else if constexpr (Rows == 3) {
         return rows[0][0] + rows[1][1] + rows[2][2];
      } else {
         value_type r = 0.0F;
         for (int i = 0; i < width; ++i)
            r += rows[i][i];
         return r;
      }
   }
   
   // [a, b, c,    [a, d, g,
   //  d, e, f, ->  b, e, h,
   //  g, h, i]     c, f, i]
   //
   // TIP: Transposing a rotation matrix is equivalent to swapping its handedness.
   //
   TEMPLATE_PARAMS
   constexpr CLASS_NAME CLASS_NAME::transposed() const noexcept requires (is_square) {
      matrix result;
      if (!std::is_constant_evaluated()) {
         if constexpr (memory_is_contiguous && std::is_same_v<value_type, float>) {
            if constexpr (Rows == 4) {
               impl::matrix_intrinsics::transpose_4x4(&rows, &result.rows);
               return result;
            }
         }
      }
      for (int v = 0; v < height; ++v)
         for (int u = 0; u < width; ++u)
            result.rows[u][v] = this->rows[v][u];
      return result;
   }
   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::transpose() noexcept requires (is_square) {
      if (!std::is_constant_evaluated()) {
         if constexpr (memory_is_contiguous && std::is_same_v<value_type, float>) {
            if constexpr (Rows == 4) {
               return impl::matrix_intrinsics::transpose_4x4(&rows);
            }
         }
      }
      constexpr int cap = height > width ? height : width;
      for (int i = 0; i < cap; ++i)
         for (int j = 0; j < i; ++j)
            std::swap(this->rows[i][j], this->rows[j][i]);
   }

   #pragma region multiply-by-matrix operator overloads
   // [ a , b , c ,     [ r , s , t ,     [ ar + bu + cx  ,  as + bv + cy  ,  at + bw + cz ,
   //   d , e , f ,  *    u , v , w ,  =    dr + eu + fx  ,  ds + ev + fy  ,  dt + ew + fz ,
   //   g , h , i ]       x , y , z ]       gr + hu + ix  ,  gs + hv + iy  ,  gt + hw + iz ]
   TEMPLATE_PARAMS
   template<size_t o>
   constexpr matrix<Rows, o, T> CLASS_NAME::operator*(const matrix<Columns, o, T>& other) { // multiply a (m,n) matrix by a (p,q) matrix; only possible if n == p
      matrix<Rows, o, T> result;
      for (int i = 0; i < height; ++i) {
         for (int j = 0; j < o; ++j) {
            result.rows[i][j] = 0.0F;
            for (int k = 0; k < width; ++k) {
               result.rows[i][j] += this->rows[i][k] * other.rows[k][j];
            }
         }
      }
      return result;
   }
   //
   TEMPLATE_PARAMS
   constexpr matrix<Rows, Columns, T>& CLASS_NAME::operator*=(const matrix<Columns, Columns, T>& other) { // multiply-assign requires that we produce an (m,n) result; given n == p, we can only multiply-assign by an (n,n) matrix
      *this = (*this) * other;
      return *this;
   }
   #pragma endregion

   #pragma region multiply-by-scalar operator overloads
   TEMPLATE_PARAMS
   template<typename OperandType>
   constexpr CLASS_NAME CLASS_NAME::operator*(OperandType operand) const noexcept requires std::is_arithmetic_v<OperandType> {
      matrix result;
      result *= operand;
      return result;
   }

   TEMPLATE_PARAMS
   template<typename OperandType>
   constexpr CLASS_NAME& CLASS_NAME::operator*=(OperandType operand) noexcept requires std::is_arithmetic_v<OperandType> {
      for (int i = 0; i < height; ++i)
         for (int j = 0; j < width; ++j)
            this->rows[i][j] *= operand;
      return *this;
   }
   #pragma endregion

   #pragma region multiply-by-column operator overload
   TEMPLATE_PARAMS
   constexpr std::array<typename CLASS_NAME::value_type, Rows> CLASS_NAME::operator*(const std::array<CLASS_NAME::value_type, Columns>& vec) const noexcept {
      //
      // This is conceptually equal to multiplying the (m,n) matrix by an (1,n) matrix, producing an (m,1) matrix.
      //
      std::array<value_type, height> result = {};
      for (int i = 0; i < height; ++i) {
         result[i] = 0.0F;
         for (int j = 0; j < width; ++j)
            result[i] += this->rows[i][j] * vec[j];
      }
      return result;
   }
   #pragma endregion
}

#undef CLASS_NAME
#undef TEMPLATE_PARAMS
#pragma pop_macro("CLASS_NAME")
#pragma pop_macro("TEMPLATE_PARAMS")
