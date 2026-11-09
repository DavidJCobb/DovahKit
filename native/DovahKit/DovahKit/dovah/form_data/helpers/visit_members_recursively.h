#pragma once
#include <array>
#include <concepts>
#include <string>
#include <tuple>
#include <type_traits>
#include "lu/concepts/instantiation_of/std_array.h"
#include "./is_visitable.h"

namespace dovah::form_data_helpers {
   namespace visit_members_recursively_impl {
      template<typename T, typename Visitor>
      concept accepted_by_visitor = requires (Visitor visitor, T& v) {
         { visitor.visit(v) };
      };

      #pragma region is_or_contains_accepted_by_visitor
         template<typename T, typename Visitor>
         struct is_or_contains_accepted_by_visitor;
         
         // Types that are directly accepted by the passed-in visitor should produce "true."
         template<typename Visitor, accepted_by_visitor<Visitor> T>
         struct is_or_contains_accepted_by_visitor<T, Visitor> : public std::true_type {};
         
         // If an array is not, itself, accepted by the visitor type, check if any subobject is.
         template<typename T, size_t S, typename Visitor>
            requires (!accepted_by_visitor<std::array<T, S>, Visitor>)
         struct is_or_contains_accepted_by_visitor<std::array<T, S>, Visitor> : public is_or_contains_accepted_by_visitor<T, Visitor> {};
         
         // If a visitable is not, itself, accepted by the visitor type, check if any subobject is.
         template<is_visitable T, typename Visitor>
            requires (!accepted_by_visitor<T, Visitor>)
         struct is_or_contains_accepted_by_visitor<T, Visitor> {
            private:
               template<typename X>
               struct check_single_member {
                  static constexpr const bool value = is_or_contains_accepted_by_visitor<X, Visitor>::value;
               };
               template<>
               struct check_single_member<void> { // account for trailing `void` in members tuple
                  static constexpr const bool value = true;
               };

               template<typename>
               struct check_members;
               template<typename... Members>
               struct check_members<std::tuple<Members...>> {
                  static constexpr const bool value = (check_single_member<Members>::value || ...);
               };
            public:
               static constexpr const bool value = check_members<typename T::member_types>::value;
         };
      #pragma endregion

      template<typename T, typename Visitor>
      concept is_or_contains_accepted_by_visitor_v = is_or_contains_accepted_by_visitor<T, Visitor>::value;
      
      #pragma region Forward declarations
         // Visitables.
         template<typename Visitor, is_visitable T>
            requires (!accepted_by_visitor<T, Visitor>)
         void _exec(T& data, Visitor&& visitor);

         // Arrays.
         template<typename Visitor, lu::concepts::instantiation_of_std_array T>
            requires (!accepted_by_visitor<T, Visitor>)
         void _exec(T& data, Visitor&& visitor);

         // Types that the passed-in visitor can handle directly.
         template<typename Visitor, accepted_by_visitor<Visitor> T>
         void _exec(T& data, Visitor&& visitor);
      #pragma endregion
      
      // Visitables.
      template<typename Visitor, is_visitable T>
         requires (!accepted_by_visitor<T, Visitor>)
      void _exec(T& data, Visitor&& visitor) {
         if constexpr (is_or_contains_accepted_by_visitor_v<T, std::decay_t<Visitor>>) {
            T::visit_fields(data, [&visitor](auto& field) {
               using field_type = std::decay_t<decltype(field)>;
               if constexpr (is_or_contains_accepted_by_visitor_v<field_type, std::decay_t<Visitor>>) {
                  _exec(field, std::forward<Visitor>(visitor));
               }
            });
         }
      }

      // Arrays.
      template<typename Visitor, lu::concepts::instantiation_of_std_array T>
         requires (!accepted_by_visitor<T, Visitor>)
      void _exec(T& data, Visitor&& visitor) {
         for (auto& item : data)
            _exec(item, std::forward<Visitor>(visitor));
      }

      // Types that the passed-in visitor can handle directly.
      template<typename Visitor, accepted_by_visitor<Visitor> T>
      void _exec(T& data, Visitor&& visitor) {
         visitor.visit(data);
      }
   }

   namespace recursive_visitor_bases {
      class ignore_primitives {
         public:
            template<std::integral T>
            void visit(T&) {}

            template<std::floating_point T>
            void visit(T&) {}

            void visit(std::string&) {}
            void visit(std::string&&) {}
            void visit(const std::string&) {}
      };
   }

   template<
      typename Visitor,
      is_visitable T
   >
   void visit_members_recursively(T& data, Visitor&& visitor) {
      visit_members_recursively_impl::_exec(data, std::forward<Visitor>(visitor));
   }
}