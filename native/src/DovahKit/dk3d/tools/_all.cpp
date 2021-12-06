#include "_all.h"

namespace {
   template<typename Argument> struct _get_pointers_functor {
      void execute(DK3D::all_tool_instances::list_type& list, size_t& index) {
         list[index] = Argument::get();
         ++index;
      }
   };
}

namespace DK3D {
   all_tool_instances::all_tool_instances() {
      size_t index = 0;
      all_tools::for_each_with_args<_get_pointers_functor>(std::forward<list_type&>(this->pointers), std::forward<size_t&>(index));
   }
}