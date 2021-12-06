#include "_all.h"

namespace {
   template<typename Argument> struct _get_pointers_functor {
      static void execute(DK3D::all_tool_instances::list_type& list, size_t& index) {
         static_assert(std::is_base_of_v<DK3D::tools::base, Argument>);
         static_assert(std::is_same_v<decltype(&Argument::get), Argument&(*)()>, "A subclass T of DK3D::tools::base should be a singleton which defines {static T& get()}.");
         list[index] = &Argument::get();
         ++index;
      }
   };
}

namespace DK3D {
   all_tool_instances::all_tool_instances() {
      size_t index = 0;
      all_tools::for_each_with_args<_get_pointers_functor>(std::forward<list_type&>(this->pointers), std::forward<size_t&>(index));
   }
   tool_id all_tool_instances::id_of(const tools::base& tool) const noexcept {
      auto& list = this->pointers;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i)
         if (list[i] == &tool)
            return i;
      return tools::id_of_none;
   }
}