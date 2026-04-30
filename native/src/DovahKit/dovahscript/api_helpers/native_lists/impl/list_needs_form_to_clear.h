#pragma once
namespace dovah::loaded_forms {
   class Form;
}

namespace dovahscript::api_helpers::native_lists::impl {
   template<typename Spec>
   concept list_needs_form_to_clear = requires(typename Spec::collection_wrapped_type& list, dovah::loaded_forms::Form& form) {
      { list.clear(form) };
   };
}