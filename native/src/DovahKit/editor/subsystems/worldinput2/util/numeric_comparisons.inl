#pragma once
#include <algorithm>
#include "./numeric_comparisons.h"

namespace dovahkit::subsystems::worldinput2::util {
   template<typename ComparandType>
   constexpr bool comparison<ComparandType>::test(comparand_type v) const {
      switch (this->op) {
         case comparison_operator::equal:   return v == this->comparand;
         case comparison_operator::greater: return v >  this->comparand;
         case comparison_operator::less:    return v <  this->comparand;
         case comparison_operator::not_equal:        return v != this->comparand;
         case comparison_operator::greater_or_equal: return v >= this->comparand;
         case comparison_operator::less_or_equal:    return v <= this->comparand;
      }
      return false;
   }

   #pragma region comparison_set
      #pragma region _normalized
         template<typename ComparandType>
         constexpr bool comparison_set<ComparandType>::_normalized::range_is_possible() const {
            if (min > max)
               return false; // inverted range
            if constexpr (!std::is_integral_v<comparand_type>) {
               if (min == max && (!can_be_min || !can_be_max)) // range is [x, x) or (x, x]
                  return false;
            }
            return true;
         }

         template<typename ComparandType>
         constexpr void comparison_set<ComparandType>::_normalized::sort_not_equals() {
            auto& list = this->must_not_equal;
            std::sort(list.begin(), list.end());
         }

         template<typename ComparandType>
         constexpr void comparison_set<ComparandType>::_normalized::process_must_equal(comparand_type v) {
            if (this->must_equal.has_value()) {
               if (this->must_equal.value() != v)
                  this->impossible = true;
               return;
            }
            this->must_equal = v;
            //
            // Check this value against our bounds:
            //
            if (v < this->min || v > this->max) {
               this->impossible = true;
               return;
            }
            if constexpr (!std::is_integral_v<comparand_type>) {
               if (
                  (v == this->min && !this->can_be_min) ||
                  (v == this->max && !this->can_be_max)
               ) {
                  this->impossible = true;
               }
            }
         }
         template<typename ComparandType>
         constexpr void comparison_set<ComparandType>::_normalized::process_must_not_equal(comparand_type v) {
            if (v < this->min) // if `v` is below the minimum allowed value, we don't care about an explicit != comparison to it
               return;
            if (v > this->max) // if `v` is above the maximum allowed value, we don't care about an explicit != comparison to it
               return;
            if constexpr (!std::is_integral_v<comparand_type>) {
               if (v == this->min && !this->can_be_min)
                  return;
               if (v == this->max && !this->can_be_max)
                  return;
            }
            for (auto prior : this->must_not_equal)
               if (prior == v)
                  return;
            this->must_not_equal.push_back(v);
         }
         template<typename ComparandType>
         constexpr void comparison_set<ComparandType>::_normalized::process_must_greater(comparand_type v) {
            if constexpr (std::is_integral_v<comparand_type>) {
               if (v == std::numeric_limits<comparand_type>::max())
                  this->impossible = true;
               else
                  this->process_must_greater_or_equal(++v);
            } else {
               if (this->min < v) {
                  this->min = v;
                  this->can_be_min = false;
               }
            }
         }
         template<typename ComparandType>
         constexpr void comparison_set<ComparandType>::_normalized::process_must_greater_or_equal(comparand_type v) {
            if (this->min < v) {
               this->min = v;
               if constexpr (!std::is_integral_v<comparand_type>) {
                  this->can_be_min = true;
               }
            }
         }
         template<typename ComparandType>
         constexpr void comparison_set<ComparandType>::_normalized::process_must_less(comparand_type v) {
            if constexpr (std::is_integral_v<comparand_type>) {
               if (v == std::numeric_limits<comparand_type>::lowest())
                  this->impossible = true;
               else
                  this->process_must_less_or_equal(--v);
            } else {
               if (this->max > v) {
                  this->max = v;
                  this->can_be_min = false;
               }
            }
         }
         template<typename ComparandType>
         constexpr void comparison_set<ComparandType>::_normalized::process_must_less_or_equal(comparand_type v) {
            if (this->max > v) {
               this->max = v;
               if constexpr (!std::is_integral_v<comparand_type>) {
                  this->can_be_max = true;
               }
            }
         }

         template<typename ComparandType>
         constexpr void comparison_set<ComparandType>::_normalized::limit_range_by_not_equals() {
            if constexpr (std::is_integral_v<comparand_type>) {
               //
               // If the range is, for example, [1, 4], but we also don't allow the value to be 
               // equal to 1 or 4, then we want to shrink the range to [2, 3].
               //
               const auto&  list = this->must_not_equal;
               const size_t size = list.size();
               for (size_t i = 0; i < size; ++i) {
                  auto v = list[i];
                  if (v == this->min)
                     ++this->min;
               }
               for (size_t i = 0; i < size; ++i) {
                  auto v = list[size - i - 1];
                  if (v == this->max)
                     --this->max;
               }
            } else {
               //
               // If the range is, for example, [1, 4], but we don't allow the value to be 1 or 
               // 4, then we want to set the range to (1, 4).
               //
               for (auto v : this->must_not_equal) {
                  if (!this->can_be_min && !this->can_be_max)
                     break;
                  if (v == this->min)
                     this->can_be_min = false;
                  else if (v == this->max)
                     this->can_be_max = false;
               }
            }
         }
      #pragma endregion
   #pragma endregion

   template<typename ComparandType>
   constexpr void comparison_set<ComparandType>::normalize() {
      _normalized info;

      for (const auto& item : this->comparisons) {
         switch (item.op) {
            case comparison_operator::equal:     info.process_must_equal(item.comparand); break;
            case comparison_operator::not_equal: info.process_must_not_equal(item.comparand); break;
            case comparison_operator::greater:   info.process_must_greater(item.comparand); break;
            case comparison_operator::less:      info.process_must_less(item.comparand); break;
            case comparison_operator::greater_or_equal: info.process_must_greater_or_equal(item.comparand); break;
            case comparison_operator::less_or_equal:    info.process_must_less_or_equal(item.comparand); break;
         }
         if (info.impossible)
            break;
      }
      this->comparisons.clear();

      if (info.impossible) {
         return;
      }
      info.sort_not_equals();
      info.limit_range_by_not_equals();
      if (!info.range_is_possible()) {
         return;
      }

      if (info.must_equal.has_value()) {
         this->comparisons.push_back(comparison{
            .op        = comparison_operator::equal,
            .comparand = info.must_equal.value()
         });
         return;
      }
      if (info.min != std::numeric_limits<comparand_type>::lowest() || !info.can_be_min) {
         auto type = info.can_be_min ? comparison_operator::greater_or_equal : comparison_operator::greater;
         //
         this->comparisons.push_back(comparison{
            .op        = type,
            .comparand = info.min
         });
      }
      if (info.max != std::numeric_limits<comparand_type>::max() || !info.can_be_max) {
         auto type = info.can_be_max ? comparison_operator::less_or_equal : comparison_operator::less;
         //
         this->comparisons.push_back(comparison{
            .op        = type,
            .comparand = info.max
         });
      }
      for (const auto v : info.must_not_equal) {
         if (v < info.min || v > info.max)
            continue;
         if constexpr (!std::is_integral_v<comparand_type>) {
            if (v == info.min && !info.can_be_min)
               continue;
            if (v == info.max && !info.can_be_max)
               continue;
         }
         this->comparisons.push_back(comparison{
            .op        = comparison_operator::not_equal,
            .comparand = v
         });
      }
   }

   template<typename ComparandType>
   constexpr bool comparison_set<ComparandType>::test(comparand_type value) const {
      if (this->comparisons.empty())
         return false;
      for (const auto& cmp : this->comparisons)
         if (!cmp.test(value))
            return false;
      return true;
   }

   template<typename ComparandType>
   comparison_set<ComparandType>& comparison_set<ComparandType>::operator&=(const comparison_set& other) {
      if (!other.comparisons.size() || !this->comparisons.size()) {
         this->comparisons.clear();
         return *this;
      }
      this->comparisons.reserve(this->comparisons.size() + other.comparisons.size());
      for (const auto& item : other.comparisons)
         this->comparisons.push_back(item);
      this->normalize();
      return *this;
   }
}
