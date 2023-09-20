#include "./tool_binding.h"
#include "editor/subsystems/worldedit/tool_system/options_union.h"

namespace {
   namespace worldedit {
      using namespace dovahkit::subsystems::worldedit;
   }

   constexpr bool options_union_can_be_no_op = true;
}

namespace dovahkit::subsystems::worldinput::util {
   tool_binding::tool_binding(const tool_binding& src) {
      *this = src;
   }
   tool_binding::tool_binding(tool_binding&& src) noexcept {
      *this = std::move(src);
   }

   tool_binding& tool_binding::operator=(const tool_binding& src) {
      if (this == &src)
         return *this;

      this->id = src.id;
      if (src.options && src.id != worldedit::tools::id_of_none) {
         const auto* src_opt = (worldedit::tools::options_union*)src.options.get();
         if (this->options) {
            auto* dst_opt = (worldedit::tools::options_union*)this->options.get();
            *dst_opt = *src_opt;
         } else {
            this->options.reset(src_opt->clone());
         }
      } else {
         this->options.reset(nullptr);
      }
      return *this;
   }
   tool_binding& tool_binding::operator=(tool_binding&& src) noexcept {
      if (this == &src)
         return *this;

      std::swap(this->id,      src.id);
      std::swap(this->options, src.options);
      return *this;
   }

   bool tool_binding::operator==(const tool_binding& other) const {
      return this->compare_fast_fields(other) || this->compare_slow_fields(other);
   }

   bool tool_binding::compare_fast_fields(const tool_binding& other) const {
      if (this == &other)
         return true;

      if (this->id != other.id)
         return false;

      return true;
   }
   bool tool_binding::compare_slow_fields(const tool_binding& other) const {
      if constexpr (!options_union_can_be_no_op) {
         //
         // This optimization doesn't work, because when reading a control scheme action from a 
         // bitstream, we blindly create an options union and allow it to potentially be a no-op.
         //
         if ((this->options == nullptr) != (other.options == nullptr))
            return false;
      }
      if (this->options && other.options) {
         const auto* a = (worldedit::tools::options_union*)this->options.get();
         const auto* b = (worldedit::tools::options_union*)other.options.get();
         if (*a != *b)
            return false;
      }

      return true;
   }
}