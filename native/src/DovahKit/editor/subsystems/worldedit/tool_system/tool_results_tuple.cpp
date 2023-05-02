#include "./tool_results_tuple.h"

namespace dovahkit::subsystems::worldedit {
   void tool_results_tuple::scale(double delta_seconds) {
      classes::for_each<[]<typename T>(self_t* self, const double delta_seconds) {
         if constexpr (impl::_tool_results_tuple::can_scale<T>) {
            if (self->has_member<T>())
               std::get<T>(*self).scale(delta_seconds);
         }
      }>(this, delta_seconds);
   }
   void tool_results_tuple::merge(const tool_results_tuple& merge_from) {
      classes::for_each<[]<typename T>(self_t* self, const self_t& source) {
         if constexpr (impl::_tool_results_tuple::can_merge<T>) {
            if (self->has_member<T>())
               std::get<T>(*self).merge(std::get<T>(source));
         }
      }>(this, merge_from);
   }
}