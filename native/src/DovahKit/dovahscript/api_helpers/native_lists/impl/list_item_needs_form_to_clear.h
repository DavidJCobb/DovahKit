#pragma once
namespace dovah::loaded_forms {
   class Form;
}

namespace dovahscript::api_helpers::native_lists::impl {
   template<typename Spec>
   concept list_item_needs_form_to_clear = requires(typename Spec::value_stored_type& item, dovah::loaded_forms::Form& form) {
      { item.clear(form) };
   };
}