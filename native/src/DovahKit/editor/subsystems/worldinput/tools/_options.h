#pragma once
#include <limits>
#include "./_base.h"
#include "./_all.h"

namespace dovahkit::subsystems::worldinput::tools {
   namespace impl::option_union {
   }

   struct option_union : impl::option_union_base {
      //
      // This is a tagged union constructed at compile-time from a cobb::class_list. It 
      // constructs the tag automatically using classes' indices in the class list, and 
      // it constructs the data portion automatically  by checking the largest size and 
      // alignment among the classes in the list.
      //
      private:
         static constexpr const size_t _required_size = ([]() {
            size_t size = 0;
            all_tools::for_each([&size]<typename argument>() {
               if constexpr (dovahkit::subsystems::worldinput::tools::tool_has_options_member_type<argument>)
                  size = std::max(size, sizeof(typename argument::options));
            });
            return size;
         })();
         static constexpr const size_t _required_alignment = ([]() {
            size_t size = 0;
            all_tools::for_each([&size]<typename argument>() {
               if constexpr (dovahkit::subsystems::worldinput::tools::tool_has_options_member_type<argument>)
                  size = (std::max)(size, std::alignment_of_v<typename argument::options>);
            });
            return size;
         })();

         alignas(_required_alignment) std::array<uint8_t, _required_size> data;

         #pragma region Data destructors
            #pragma region Metaprogramming
            template<typename T> static void _typed_data_destructor(option_union& ou) {
               if constexpr (dovahkit::subsystems::worldinput::tools::tool_has_options_member_type<T>) {
                  using Options = typename T::options;
                  Options* p = ou.as<T>();
                  p->~Options();
               }
            }
            //
            template<typename T> struct _types_to_destructors;
            template<typename... Types> struct _types_to_destructors<std::tuple<Types...>> {
               static constexpr const auto value = std::array{ &_typed_data_destructor<Types>... };
            };
            //
            static constexpr const auto union_data_destructors_by_id = _types_to_destructors<dovahkit::subsystems::worldinput::all_tools::as_tuple>::value;
            #pragma endregion

         void _destroy_data();
         #pragma endregion

         constexpr const option_union* _as_const() const noexcept { return this; }

      public:
         option_union() : impl::option_union_base(id_of_none), data({}) {}
         template<typename T> requires is_tool_options<T> option_union(const T& src) : impl::option_union_base(id_of_tool_options<T>()) {
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

         // Test type (self and instance)
         template<is_tool_or_options T> bool is() const noexcept;
         template<is_tool_or_options T> static bool is(const option_union_base& b);

         // Cast self
         template<tools::tool_has_options_member_type Tool> const typename Tool::options* as() const noexcept;
         template<is_tool_options Options> const Options* as() const noexcept;

         // Cast self (non-const)
         template<tools::tool_has_options_member_type Tool> typename Tool::options* as() noexcept;
         template<is_tool_options Options> Options* as() noexcept;

         // Static; cast instance
         template<tools::tool_has_options_member_type Tool> static const typename Tool::options* as(const option_union_base& b);
         template<is_tool_options Options> static const Options* as(const option_union_base& b);

         // Static; cast instance (non-const)
         template<tools::tool_has_options_member_type Tool> static typename Tool::options* as(option_union_base& b);
         template<is_tool_options Options> static Options* as(option_union_base& b);

         static option_union construct_for_type(tool_id);
   };
}

#include "_options.inl"