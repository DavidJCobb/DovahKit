#include "./collection_ordered_children.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_addenda.h"
#include "dovahscript/api_helpers/native_lists/member_function_spec.h"
#include "dovahscript/api_helpers/native_lists/common/pull_collection.h"
#include "dovahscript/api_helpers/native_lists/define_metatable.h"
#include "dovahscript/wrapper.h"

namespace {
   using namespace dovahscript;
   using wrapper_spec = dovahscript::wrapper_likes::native_lists::form_ordered_children;

   struct member_function_spec : public api_helpers::native_lists::member_function_spec {
      using collection_wrapped_type = const std::vector<dovah::form_stub*>;
      using value_stored_type       = dovah::form_stub*;
      using value_working_type      = dovah::form_stub*;

      static constexpr const bool allow_insertions_past_end = false;
      static constexpr const bool allow_removals = false;

      static constexpr const auto pull_collection = &api_helpers::native_lists::common::pull_collection<wrapper_spec::metatable_key>;

      static const collection_wrapped_type* unwrap_collection(wrapper& self) {
         auto* stub = self.stub;
         if (!stub)
            return nullptr;
         auto* addenda = self.stub->addenda;
         if (!addenda)
            return nullptr;
         return &addenda->ordered_children.get_active_list();
      }
   };

   // Ordered-child lists shouldn't allow direct editing. The owning form can expose member functions 
   // to manipulate/edit the child list in whatever manner actually makes sense for that form type.
   static_assert(!api_helpers::native_lists::impl::concepts::can_assign_elements<member_function_spec>, "This collection should not be directly mutable.");
}

namespace dovahscript::wrapper_likes::native_lists {
   /*static*/ void wrapper_spec::define_metatable(lua_State* L) {
      api_helpers::native_lists::define_metatable<metatable_key, class_name, member_function_spec>(L);
   }
   /*static*/ int wrapper_spec::push(lua_State* L, const wrapper& parent) {
      wrapper out = parent;
      out.append_part(signature);
      out.is_collection = true;
      return core::subsystems::userdata::get().push(L, out, metatable_key.data());
   }
}