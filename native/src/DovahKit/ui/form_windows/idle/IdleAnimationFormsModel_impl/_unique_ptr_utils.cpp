#include "./_unique_ptr_utils.h"
#include "./action_node.h"
#include "./graph_node.h"
#include "./idle_node.h"

//
// Using std::unique_ptr with forward-declared classes is... unreliable. The 
// template instantiation's destructor needs to be able to delete the class 
// that you're wrapping in a smart pointer, and if that class has only been 
// forward-declared by the time the destructor is generated, then it'll be 
// incomplete and thus impossible to delete.
// 
// There *is* a way to avoid needing a custom deleter if you're super careful 
// with your forward-declaration and include order, but it's fiddly and not 
// worth trying to figure out.
//

namespace IdleAnimationFormsModel_impl {
   #define OVERLOAD(T) void node_deleter::operator()(T* n) const noexcept { delete n; }
   OVERLOAD(action_node);
   OVERLOAD(graph_node);
   OVERLOAD(idle_node);
   OVERLOAD(loose_action_parent_node);
   OVERLOAD(loose_idle_parent_node);
}