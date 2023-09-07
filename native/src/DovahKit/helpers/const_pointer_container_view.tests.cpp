#include "./const_pointer_container_view.h"

#include <vector>
static_assert(
   []() {
      std::vector<int>  foo{ 1, 2, 3, 4 };
      std::vector<int*> bar{ &foo[0], &foo[1], &foo[2], &foo[3] };

      auto  view = cobb::const_pointer_container_view(bar);
      auto  b = view.begin();
      auto* c = *b;

      return true;
   }(),
   "failed"
);