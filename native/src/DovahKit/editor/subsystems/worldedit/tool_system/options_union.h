#pragma once
#include <algorithm> // std::max
#include <memory> // std::construct_at, std::destroy_at
#include "helpers/function_pointer.h"
#include "./tools/_all.h"
#include "./concepts/is_tool_options.h"
#include "./concepts/tool_or_tool_options.h"
#include "./utils/all_tools_with_options.h"
#include "./id_of.h"
#include "./opaque_options_union.h"

namespace dovahkit::subsystems::worldedit::tools {
   class options_union : public opaque_options_union {
      //
      // This is a tagged union constructed at compile-time from a cobb::class_list. It 
      // constructs the tag automatically using classes' indices in the class list, and 
      // it constructs the data portion automatically  by checking the largest size and 
      // alignment among the classes in the list.
      //
      private:
         static constexpr const size_t _required_size      = all_tools_with_options::reduce<size_t, []<typename Current>(size_t prev) constexpr -> size_t {
            return (std::max)(prev, sizeof(typename Current::options));
         }>();
         static constexpr const size_t _required_alignment = all_tools_with_options::reduce<size_t, []<typename Current>(size_t prev) constexpr -> size_t {
            return (std::max)(prev, std::alignment_of_v<typename Current::options>);
         }>();

         alignas(_required_alignment) std::array<uint8_t, _required_size> data;

         template<tool_with_options_member_type T> static void _typed_destructor(options_union& ou) {
            std::destroy_at((typename T::options*)ou.data.data());
         }
         template<tool_with_options_member_type T> static void _typed_copy_construct(const options_union& src, options_union& dst) {
            std::construct_at((typename T::options*)dst.data.data(), std::as_const(src.as<typename T::options>()));
         }
         template<tool_with_options_member_type T> static void _typed_default_construct(options_union& dst) {
            std::construct_at((typename T::options*)dst.data.data());
         }

         struct _type_table_entry {
            tool_id id = id_of_none;
            cobb::function_pointer<void(options_union&)> destruct  = nullptr;
            cobb::function_pointer<void(options_union&)> construct = nullptr;
            cobb::function_pointer<void(const options_union&, options_union&)> construct_copy = nullptr;
         };
         static constexpr const auto _type_table = [](){
            std::array<_type_table_entry, all_tools_with_options::count> entries = {};
            {
               size_t i = 0;
               all_tools_with_options::for_each([&entries, &i]<typename Tool>() {
                  auto& entry = entries[i];
                  entry.id             = id_of<Tool>;
                  entry.destruct       = &_typed_destructor<Tool>;
                  entry.construct      = &_typed_default_construct<Tool>;
                  entry.construct_copy = &_typed_copy_construct<Tool>;
                  ++i;
               });
            }
            return entries;
         }();

         void _destroy_data();

      public:
         constexpr options_union() : opaque_options_union(id_of_none), data({}) {}

         template<is_tool_options T>
         explicit options_union(const T& src) : opaque_options_union(id_of<T>) {
            std::construct_at((T*)this->data.data(), src);
         }

         options_union(const options_union& o) : opaque_options_union(o.tag) {
            memcpy(this->data.data(), &o.data, this->data.size());
         }

         ~options_union() {
            this->_destroy_data();
         }

         options_union& operator=(const options_union& o) {
            this->_destroy_data();
            this->tag = o.tag;
            for (const auto& entry : _type_table) {
               if (entry.id == o.tag) {
                  entry.construct_copy(o, *this);
                  break;
               }
            }
            return *this;
         }
         options_union& operator=(options_union&& o) noexcept {
            this->_destroy_data();
            this->tag = o.tag;
            for (const auto& entry : _type_table) {
               if (entry.id == o.tag) {
                  entry.construct_copy(o, *this);
                  break;
               }
            }
            return *this;
         }

         // Test type (self and instance)
         template<tool_or_tool_options T> constexpr bool is() const noexcept;

         // Cast self
         template<tool_with_options_member_type Tool> const typename Tool::options& as() const noexcept;
         template<is_tool_options Options> const Options& as() const noexcept;

         // Cast self (non-const)
         template<tool_with_options_member_type Tool> typename Tool::options& as() noexcept;
         template<is_tool_options Options> Options& as() noexcept;

         static options_union construct_for_type(tool_id);

         options_union* clone() const;
   };
}

#include "options_union.inl"