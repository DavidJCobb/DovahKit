#pragma once
#include <array>
#include <concepts>
#include <string>
#include <tuple>
#include <type_traits>
#include "lu/concepts/instantiation_of/std_array.h"
#include "./is_subobject_vector_type.h"
#include "./is_visitable.h"

namespace dovah::form_data::helpers {
   // Check whether a visitor type can act directly on a given value type (as opposed to 
   // us recursively walking that value pair for it until we find something it can act on).
   template<typename T, typename U, typename Visitor>
   concept can_visit_recursively_in_tandem_with = requires (Visitor visitor, T&& a, U&& b) {
      { visitor.visit(a, b) };
   };

   namespace visit_members_in_tandem_recursively_impl {
      #pragma region is_or_contains_accepted_by_visitor
         template<typename T, typename U, typename Visitor>
         struct is_or_contains_accepted_by_visitor;
         
         // Types that are directly accepted by the passed-in visitor should produce "true."
         template<typename Visitor, typename T, typename U>
            requires can_visit_recursively_in_tandem_with<T, U, Visitor>
         struct is_or_contains_accepted_by_visitor<T, U, Visitor> : public std::true_type {};
         
         // If an array is not, itself, accepted by the visitor type, check if any subobject is.
         template<typename Visitor, lu::concepts::instantiation_of_std_array T, lu::concepts::instantiation_of_std_array U>
            requires (!can_visit_recursively_in_tandem_with<T, U, Visitor> && std::tuple_size_v<T> == std::tuple_size_v<U>)
         struct is_or_contains_accepted_by_visitor<T, U, Visitor> : public is_or_contains_accepted_by_visitor<typename T::value_type, typename U::value_type, Visitor> {};

         // If a vector is not, itself, accepted by the visitor type, check if any subobject is.
         template<typename Visitor, is_subobject_vector_type T, is_subobject_vector_type U>
            requires (!can_visit_recursively_in_tandem_with<T, U, Visitor>)
         struct is_or_contains_accepted_by_visitor<T, U, Visitor> : public is_or_contains_accepted_by_visitor<typename T::value_type, typename U::value_type, Visitor> {};
         
         // If a visitable is not, itself, accepted by the visitor type, check if any subobject is.
         template<is_visitable T, is_visitable U, typename Visitor>
            requires (!can_visit_recursively_in_tandem_with<T, U, Visitor>)
         struct is_or_contains_accepted_by_visitor<T, U, Visitor> {
            private:
               template<typename X, typename Y>
               struct check_single_member {
                  static constexpr const bool value = is_or_contains_accepted_by_visitor<X, Y, Visitor>::value;
               };
               template<>
               struct check_single_member<void, void> { // account for trailing `void` in members tuple
                  static constexpr const bool value = true;
               };

               template<typename, typename>
               struct check_members;
               template<typename... MembersT, typename... MembersU>
               struct check_members<std::tuple<MembersT...>, std::tuple<MembersU...>> {
                  static constexpr const bool value = (check_single_member<MembersT, MembersU>::value || ...);
               };
            public:
               static constexpr const bool value = check_members<typename T::member_types, typename U::member_types>::value;
         };
      #pragma endregion

      template<typename T, typename U, typename Visitor>
      concept is_or_contains_accepted_by_visitor_v = is_or_contains_accepted_by_visitor<T, U, Visitor>::value;
      
      #pragma region Forward declarations
         // Visitables.
         template<typename Visitor, typename T, typename U>
            requires (
               !can_visit_recursively_in_tandem_with<T, U, Visitor>
            && is_visitable<std::decay_t<T>>
            && is_visitable<std::decay_t<U>>
            )
         void _exec(T&&, U&&, Visitor&&);

         // Arrays.
         template<typename Visitor, lu::concepts::instantiation_of_std_array T, lu::concepts::instantiation_of_std_array U>
            requires (
               !can_visit_recursively_in_tandem_with<T, U, Visitor>
            && lu::concepts::instantiation_of_std_array<T>
            && lu::concepts::instantiation_of_std_array<U>
            && std::tuple_size_v<T> == std::tuple_size_v<U>
            )
         void _exec(T&&, U&&, Visitor&&);

         // Vectors.
         template<typename Visitor, is_subobject_vector_type T, is_subobject_vector_type U>
            requires (!can_visit_recursively_in_tandem_with<T, U, Visitor>)
         void _exec(T&&, U&&, Visitor&&);

         // Types that the passed-in visitor can handle directly.
         template<typename Visitor,typename T, typename U>
            requires can_visit_recursively_in_tandem_with<T, U, Visitor>
         void _exec(T&&, U&&, Visitor&&);
      #pragma endregion
      
      // Visitables.
      template<typename Visitor, typename T, typename U>
         requires (
            !can_visit_recursively_in_tandem_with<T, U, Visitor>
         && is_visitable<std::decay_t<T>>
         && is_visitable<std::decay_t<U>>
         )
      void _exec(T&& a, U&& b, Visitor&& visitor) {
         if constexpr (is_or_contains_accepted_by_visitor_v<T, U, std::decay_t<Visitor>>) {
            T::visit_fields_in_tandem(a, b, [&visitor](auto& field_a, auto& field_b) {
               using field_type_a = std::decay_t<decltype(field_a)>;
               using field_type_b = std::decay_t<decltype(field_b)>;
               if constexpr (is_or_contains_accepted_by_visitor_v<field_type_a, field_type_b, std::decay_t<Visitor>>) {
                  _exec(field_a, field_b, std::forward<Visitor>(visitor));
               }
            });
         }
      }

      // Arrays.
      template<typename Visitor, lu::concepts::instantiation_of_std_array T, lu::concepts::instantiation_of_std_array U>
         requires (
            !can_visit_recursively_in_tandem_with<T, U, Visitor>
         && lu::concepts::instantiation_of_std_array<T>
         && lu::concepts::instantiation_of_std_array<U>
         && std::tuple_size_v<T> == std::tuple_size_v<U>
         )
      void _exec(T&& a, U&& b, Visitor&& visitor) {
         constexpr const size_t size = std::tuple_size_v<T>;
         for (size_t i = 0; i < size; ++i)
            _exec(a[i], b[i], std::forward<Visitor>(visitor));
      }

      // Vectors.
      template<typename Visitor, is_subobject_vector_type T, is_subobject_vector_type U>
         requires (!can_visit_recursively_in_tandem_with<T, U, Visitor>)
      void _exec(T&& a, U&& b, Visitor&& visitor) {
         const size_t size_a   = a.size();
         const size_t size_b   = b.size();
         const size_t size_min = (size_a < size_b) ? size_a : size_b;
         for (size_t i = 0; i < size_min; ++i)
            _exec(a[i], b[i], std::forward<Visitor>(visitor));
      }

      // Types that the passed-in visitor can handle directly.
      template<typename Visitor, typename T, typename U>
         requires can_visit_recursively_in_tandem_with<T, U, Visitor>
      void _exec(T&& a, U&& b, Visitor&& visitor) {
         visitor.visit(a, b);
      }
   }

   template<typename Visitor, typename T, typename U>
      requires (is_visitable<std::decay_t<T>> && is_visitable<std::decay_t<U>>)
   void visit_members_in_tandem_recursively(T&& a, U&& b, Visitor&& visitor) {
      visit_members_in_tandem_recursively_impl::_exec(a, b, std::forward<Visitor>(visitor));
   }
}