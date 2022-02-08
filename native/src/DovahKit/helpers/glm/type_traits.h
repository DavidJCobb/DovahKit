#pragma once
#include <glm/glm.hpp>
#include <type_traits>

namespace cobb::glm {
   namespace impl {
      template<typename T> struct is_vec {
         static constexpr bool value = false;
      };
      template<::glm::length_t N, typename F, ::glm::qualifier Q> struct is_vec<typename ::glm::vec<N, F, Q>> {
         static constexpr bool value = true;
      };
   }
   template<typename T> constexpr bool is_vec = impl::is_vec<T>::value;

   template<typename T> struct vec_traits;
   template<::glm::length_t N, typename F, ::glm::qualifier Q> struct vec_traits<typename ::glm::vec<N, F, Q>> {
      using type = ::glm::vec<N, F, Q>;

      static constexpr ::glm::length_t  axes = N;
      static constexpr ::glm::qualifier precision = Q;
      using value_type = F;

      static constexpr ::glm::length_t size = N;

      static constexpr bool is_contiguous = sizeof(type) == sizeof(F) * N;
   };

   namespace impl {
      template<typename T> struct vec_length;
      template<::glm::length_t N, typename F, ::glm::qualifier Q> struct vec_length<typename ::glm::vec<N, F, Q>> {
         static constexpr ::glm::length_t value = N;
      };
   }
   template<typename T> requires is_vec<T> constexpr ::glm::length_t vec_length = vec_traits<T>::axes;
   
   //

   namespace impl {
      template<typename T> struct is_mat {
         static constexpr bool value = false;
      };
      template<::glm::length_t C, ::glm::length_t R, typename F, ::glm::qualifier Q> struct is_mat<typename ::glm::mat<C, R, F, Q>> {
         static constexpr bool value = true;
      };
   }
   template<typename T> constexpr bool is_mat = impl::is_mat<T>::value;

   template<typename T> struct mat_traits;
   template<::glm::length_t C, ::glm::length_t R, typename F, ::glm::qualifier Q> struct mat_traits<typename ::glm::mat<C, R, F, Q>> {
      using type = ::glm::mat<C, R, F, Q>;

      static constexpr ::glm::length_t  cols = C;
      static constexpr ::glm::length_t  rows = R;
      static constexpr ::glm::qualifier precision = Q;
      using value_type = F;

      static constexpr ::glm::length_t size = C * R;

      static constexpr bool is_contiguous = sizeof(type) == sizeof(F) * (C * R);
   };
}
