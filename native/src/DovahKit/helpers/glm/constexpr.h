#pragma once
#include <glm/glm.hpp>
#include "../math/sqrt.h"
#include "./type_traits.h"
#include "../unreachable.h"

//
// Wrappers around GLM library functions, to allow usage during constant evaluation.
//

namespace cobb::glm {
	template<typename Vec> requires (is_vec<Vec> && vec_traits<Vec>::axes == 3)
	constexpr Vec cross(const Vec& a, const Vec& b) {
		static_assert(std::numeric_limits<typename Vec::value_type>::is_iec559, "glm::cross requires a floating-point value type");
		if (std::is_constant_evaluated()) {
			return Vec(
				a.y * b.z - b.y * a.z,
				a.z * b.x - b.z * a.x,
				a.x * b.y - b.x * a.y
			);
		} else {
			return ::glm::cross(a, b);
		}
	}

	template<typename Vec> requires (is_vec<Vec>)
	constexpr typename Vec::value_type dot(const Vec& a, const Vec& b) {
		static_assert(std::numeric_limits<typename Vec::value_type>::is_iec559, "glm::cross requires a floating-point value type");
		if (std::is_constant_evaluated()) {
			constexpr size_t axis_count = vec_traits<Vec>::axes;
			if (axis_count < 1 || axis_count > 4) {
				cobb::unreachable();
			}
			auto c = a * b;

			typename Vec::value_type v = 0;
			for (int i = 0; i < axis_count; i++)
				v += c[i];
			return v;
		} else {
			return ::glm::dot(a, b);
		}
	}

	template<typename Vec> requires (is_vec<Vec>)
	constexpr float length(const Vec& v) {
		static_assert(std::numeric_limits<typename Vec::value_type>::is_iec559, "glm::normalize requires a floating-point value type");
		if (std::is_constant_evaluated()) {
			constexpr size_t axis_count = vec_traits<Vec>::axes;

			float length = 0;
			for (size_t i = 0; i < axis_count; ++i) {
				auto n = v[i];
				length += (n * n);
			}
			return cobb::sqrt(length);
		} else {
			return ::glm::length(v);
		}
	}

	template<typename Vec> requires (is_vec<Vec>)
	constexpr Vec normalize(const Vec& v) {
		static_assert(std::numeric_limits<typename Vec::value_type>::is_iec559, "glm::normalize requires a floating-point value type");
		if (std::is_constant_evaluated()) {
			constexpr size_t axis_count = vec_traits<Vec>::axes;

			float length = 0;
			for (size_t i = 0; i < axis_count; ++i) {
				auto n = v[i];
				length += (n * n);
			}
			length = cobb::sqrt(length);

			Vec result = {};
			for (size_t i = 0; i < axis_count; ++i) {
				result[i] = v[i] / length;
			}
			return result;
		} else {
			return ::glm::normalize(v);
		}
	}
}