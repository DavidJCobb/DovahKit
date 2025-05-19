#pragma once
#include <algorithm> // std::max
#include <variant>
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"
#include "helpers/function_pointer.h"
#include "./tools/_all.h"
#include "./concepts/is_tool_options.h"
#include "./concepts/tool_or_tool_options.h"
#include "./utils/all_tools_with_options.h"
#include "./id_of.h"

namespace dovahkit::subsystems::worldedit::tools {
   class options_union : public
      cobb::tuples::unpack_types_into<
         all_tool_options::as_tuple,
         std::variant
      >
   {
      public:
         template<tool_or_tool_options T>
         constexpr bool is() const noexcept {
            if constexpr (is_tool_options<T>) {
               return std::holds_alternative<T>(*this);
            } else {
               return std::holds_alternative<typename T::options>(*this);
            }
         }

         template<tool_with_options_member_type Tool> const typename Tool::options& as() const noexcept {
            return std::get<typename Tool::options>(*this);
         }
         template<is_tool_options Options> const Options& as() const noexcept {
            return std::get<Options>(*this);
         }
         //
         template<tool_with_options_member_type Tool> typename Tool::options& as() noexcept {
            return std::get<typename Tool::options>(*this);
         }
         template<is_tool_options Options> Options& as() noexcept {
            return std::get<Options>(*this);
         }

         static options_union construct_for_type(tool_id);
         static constexpr bool tool_has_options(tool_id id) {
            return all_tools_with_options::for_each_until_true([id]<typename Tool>() {
               if constexpr (!tool_with_options_member_type<Tool>)
                  return false;
               return id_of<Tool> == id;
            });
         }

         options_union* clone() const;

         constexpr void stream(cobb::bitstreams::reader&);
         constexpr void stream(cobb::bitstreams::writer&) const;
   };
}

#include "options_union.inl"