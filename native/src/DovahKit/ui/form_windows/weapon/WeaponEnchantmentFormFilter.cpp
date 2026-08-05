#include "./WeaponEnchantmentFormFilter.h"
#include "dovah/form_stub.h"
#include "editor/subsystems/form_info_cache/core.h"
#include "editor/subsystems/form_info_cache/cached_data/by_form_type/enchantment.h"

/*virtual*/ bool WeaponEnchantmentFormFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   if (stub.form_type != dovah::form_type::enchantment)
      return false;

   auto* info = dovahkit::subsystems::form_info_cache::core::get().get_enchantment_info(stub);
   if (this->_is_staff) {
      return info->enchantment_type == dovah::magic_spell_type::enchantment_staves;
   }
   return info->enchantment_type == dovah::magic_spell_type::enchantment_normal;
}

void WeaponEnchantmentFormFilter::setWeaponIsStaff(bool v) {
   if (v == this->_is_staff)
      return;
   this->_is_staff = v;
   this->_refilter_all_forms();
}