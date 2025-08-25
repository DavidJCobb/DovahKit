#include "./PackageTemplatePickerFilter.h"
#include "dovah/data/packages/legacy_type.h"
#include "dovah/form_stub.h"
#include "editor/subsystems/form_info_cache/cached_data/by_form_type/package.h"
#include "editor/subsystems/form_info_cache/core.h"

/*virtual*/ bool PackageTemplatePickerFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   auto& fic  = dovahkit::subsystems::form_info_cache::core::get();
   auto* info = fic.get_package_info(stub);
   if (!info)
      return false;
   return info->legacy_type == dovah::packages::legacy_type::custom_template;
}