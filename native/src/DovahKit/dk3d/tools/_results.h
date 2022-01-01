#pragma once
#include "_base.h"
#include "_all.h"

namespace DK3D {
   class combined_tool_results;
   namespace impl::_combined_tool_results {
      // ugh, MSVC2019 doesn't support requires expressions...
      template<typename T> concept can_scale = requires(T x) { { x.scale(0.0) }; };
      template<typename T> concept can_merge = requires(combined_tool_results& x, const combined_tool_results& y) { { x.merge(std::forward<const combined_tool_results&>(y)) }; };
   }
   // Struct containing one member for each tool's results type.
   class combined_tool_results : public all_tool_results::as_tuple {
      private:
         using self_t  = combined_tool_results;
         using classes = all_tool_results;

         template<typename T> struct _foreach_scale {
            static void execute(self_t* self, double delta_seconds) {
               if constexpr (impl::_combined_tool_results::can_scale<T>) {
                  std::get<T>(*self).scale(delta_seconds);
               }
            }
         };
         template<typename T> struct _foreach_merge {
            static void execute(self_t* self, const self_t& source) {
               if constexpr (impl::_combined_tool_results::can_merge<T>) {
                  std::get<T>(*self).merge(std::get<T>(source));
               }
            }
         };

         template<typename T> requires (all_tools_with_results::contains<T> || all_tool_results::contains<T>)
         using _to_results = std::conditional_t<all_tools_with_results::contains<T>, typename T::results, T>;

      public:
         using tuple::tuple; // inherit constructor, etc.

         // alternative to std::get which accepts a tool class or a tool::results struct
         template<typename T> requires (all_tools_with_results::contains<T> || all_tool_results::contains<T>)
         _to_results<T>& get_member() {
            return std::get<_to_results<T>>(*this);
         }

         // alternative to std::get-and-then-assign which accepts a tool class or a tool::results struct; 
         // templated type must be specified explicitly
         template<typename T> requires (all_tools_with_results::contains<T> || all_tool_results::contains<T>)
         void set_member(const _to_results<T>& v) {
            std::get<_to_results<T>>(*this) = v;
         }

         // alternative to std::get-and-then-assign which accepts a tool::results struct as an argument
         template<typename A> requires (all_tool_results::contains<A>)
         void set_member(const A& v) {
            std::get<A>(*this) = v;
         }

         template<typename A> requires (all_tool_results::contains<A>)
         void merge_member(const A& v) {
            if constexpr (impl::_combined_tool_results::can_merge<A>) {
               std::get<A>(*this).merge(v);
            } else {
               std::get<A>(*this) = v;
            }
         }

         void scale(double delta_seconds) {
            classes::for_each_with_args<_foreach_scale>(this, delta_seconds);
         }
         void merge(const combined_tool_results& merge_from) {
            classes::for_each_with_args<_foreach_merge>(this, merge_from);
         }
   };
}