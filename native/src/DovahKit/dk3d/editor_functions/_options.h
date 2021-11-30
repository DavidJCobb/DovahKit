#pragma once
#include <limits>
#include "_base.h"
#include "_all.h"

namespace DK3D::editor_functions {
   struct option_union : impl::option_union_base {
      static_assert(empty_tag >= all_editor_functions::count, "The option_union_base::tag field isn't large enough to accommodate all editor functions. Use a larger numeric type.");
      private:
         template<typename T> struct _find {
            //
            // We need to be able to use cobb::class_list::index_of_matching to find 
            // the editor_function class that has a given options type. However, the 
            // functors we pass to index_of_matching can only be templated on the 
            // type currently being iterated... so we'll use nested structs along 
            // with nested templating. If, given some type T, you want to find the 
            // editor function class whose options type is T, then you'd want to 
            // pass (_find<T>::functor) to index_of_matching.
            //
            template<typename argument> struct functor {
               static consteval bool execute() {
                  return std::is_same_v<T, argument::options>;
               }
            };
         };

      public:
         option_union() : impl::option_union_base(empty_tag) {}
         template<typename T> option_union(const T& src) : impl::option_union_base(all_editor_functions::index_of_matching<_find<T>::functor>()) {
            memcpy(&this->data, &src, sizeof(T));
         }
         option_union(const option_union& o) : impl::option_union_base(o.tag) {
            memcpy(&this->data, &o.data, sizeof(data));
         }

         template<typename T> inline bool is() const noexcept {
            constexpr auto desired_tag = all_editor_functions::index_of_matching<_find<T>::functor>();
            return this->tag == desired_tag;
         }
         template<typename T> inline static bool is(const option_union_base& b) {
            return ((const option_union*)&b)->is<T>();
         }

         template<typename T> inline const T* as() const noexcept {
            if (!is<T>())
               return nullptr;
            return (T*)&this->data;
         }
         template<typename T> inline static const T* as(const option_union_base& b) {
            return ((const option_union*)&b)->as<T>();
         }

         union data_union {
            data_union() : none(0) {}

            int none : 1 = 0; // dummy, for init
            move_camera::options o_move_camera;
         } data;
   };
}
