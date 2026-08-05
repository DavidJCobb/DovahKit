#pragma once
#include "widgets/widget-data/DKCustomFormFilter.h"

class WeaponEnchantmentFormFilter final : public DKCustomFormFilter {
   public:
      using DKCustomFormFilter::DKCustomFormFilter;

      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

   public:
      void setWeaponIsStaff(bool);

   protected:
      bool _is_staff = false;
};