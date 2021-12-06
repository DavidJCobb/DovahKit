#pragma once
#include <limits>
#include "_base.h"
#include "_all.h"

namespace DK3D::tools {
   struct option_union : impl::option_union_base {
      //
      // This is a tagged union constructed at compile-time from a cobb::class_list. It 
      // constructs the tag automatically using classes' indices in the class list, and 
      // it constructs the data portion automatically  by checking the largest size and 
      // alignment among the classes in the list.
      //
      private:
         template<typename argument> struct alignof_functor {
            static constexpr size_t execute(size_t prev) {
               return std::max(prev, std::alignment_of_v<argument::options>);
            }
         };
         template<typename argument> struct sizeof_functor {
            static constexpr size_t execute(size_t prev) {
               return std::max(prev, sizeof(argument::options));
            }
         };
         static constexpr size_t _required_size      = all_tools::for_each_with_previous<sizeof_functor>(size_t(0));
         static constexpr size_t _required_alignment = all_tools::for_each_with_previous<alignof_functor>(size_t(0));

         alignas(_required_alignment) std::array<uint8_t, _required_size> data;

         void _destroy_data();

      public:
         option_union() : impl::option_union_base(id_of_none) {}
         template<typename T> requires is_tool_options<T> option_union(const T& src) : impl::option_union_base(id_of_tool<T>()) {
            memcpy(this->data.data(), &src, sizeof(T));
         }
         option_union(const option_union& o) : impl::option_union_base(o.tag) {
            memcpy(this->data.data(), &o.data, this->data.size());
         }

         ~option_union() {
            this->_destroy_data();
         }

         option_union& operator=(const option_union& o) {
            this->_destroy_data();
            this->tag = o.tag;
            memcpy(this->data.data(), o.data.data(), this->data.size());
            return *this;
         }
         option_union& operator=(option_union&& o) noexcept {
            std::swap(this->tag,  o.tag);
            std::swap(this->data, o.data);
            return *this;
         }

         template<typename T> requires (id_of_tool<T>() != id_of_none) inline bool is() const noexcept {
            return this->tag == id_of_tool<T>();
         }
         template<typename T> requires (id_of_tool<T>() != id_of_none) inline static bool is(const option_union_base& b) {
            return ((const option_union*)&b)->is<T>();
         }

         template<typename T> requires (id_of_tool<T>() != id_of_none) inline const T* as() const noexcept {
            if (!is<T>())
               return nullptr;
            return (T*)this->data.data();
         }
         template<typename T> requires (id_of_tool<T>() != id_of_none) inline static const T* as(const option_union_base& b) {
            return ((const option_union*)&b)->as<T>();
         }
         template<typename T> requires (id_of_tool<T>() != id_of_none) inline T* as() noexcept {
            if (!is<T>())
               return nullptr;
            return (T*)this->data.data();
         }
         template<typename T> requires (id_of_tool<T>() != id_of_none) inline static T* as(option_union_base& b) {
            return ((const option_union*)&b)->as<T>();
         }

         static option_union construct_for_type(tool_id);
   };
}
