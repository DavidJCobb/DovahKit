#pragma once

namespace dovahscript::api_helpers::native_lists::impl::fields::prepare_for_insertion {
   template<typename Spec>
   concept present = requires {
      { Spec::prepare_for_insertion };
   };
}