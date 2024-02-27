#pragma once

namespace dovahkit::subsystems::form_info_cache {
   enum class cacheable_trait {
      attached_scripts,
      model_path,
      quest_filter,


      __count
   };
   static constexpr const size_t cacheable_trait_count = (size_t)cacheable_trait::__count;
}