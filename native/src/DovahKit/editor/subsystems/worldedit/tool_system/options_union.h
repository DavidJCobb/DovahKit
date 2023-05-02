#pragma once
#include "./tools/_all.h"
#include "./concepts/is_tool_options.h"
#include "./concepts/tool_or_tool_options.h"
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
         static constexpr const size_t _required_size = ([]() {
            size_t size = 0;
            all_tools::for_each([&size]<typename Argument>() {
               if constexpr (tool_with_options_member_type<Argument>)
                  size = std::max(size, sizeof(Argument::options));
            });
            return size;
         })();
         static constexpr const size_t _required_alignment = ([]() {
            size_t size = 0;
            all_tools::for_each([&size]<typename Argument>() {
               if constexpr (tool_with_options_member_type<Argument>)
                  size = (std::max)(size, std::alignment_of_v<Argument::options>);
            });
            return size;
         })();

         alignas(_required_alignment) std::array<uint8_t, _required_size> data;

         #pragma region Data destructors
            #pragma region Metaprogramming
            template<typename T> static void _typed_data_destructor(options_union& ou) {
               if constexpr (tool_with_options_member_type<T>) {
                  using Options = T::options;
                  Options& p = ou.as<T>();
                  p.~Options();
               }
            }
            //
            template<typename T> struct _types_to_destructors;
            template<typename... Types> struct _types_to_destructors<std::tuple<Types...>> {
               static constexpr const auto value = std::array{ &_typed_data_destructor<Types>... };
            };
            //
            static constexpr const auto union_data_destructors_by_id = _types_to_destructors<all_tools::as_tuple>::value;
            #pragma endregion

         void _destroy_data();
         #pragma endregion

      public:
         constexpr options_union() : opaque_options_union(id_of_none), data({}) {}

         template<is_tool_options T>
         options_union(const T& src) : opaque_options_union(id_of<T>()) {
            memcpy(this->data.data(), &src, sizeof(T));
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
            memcpy(this->data.data(), o.data.data(), this->data.size());
            return *this;
         }
         options_union& operator=(options_union&& o) noexcept {
            std::swap(this->tag,  o.tag);
            std::swap(this->data, o.data);
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
   };
}

#include "options_union.inl"