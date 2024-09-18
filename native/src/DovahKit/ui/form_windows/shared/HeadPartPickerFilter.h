#pragma once
#include <optional>
#include "dovah/data/headparts.h"
#include "dovah/data/sex.h"
#include "widgets/widget-data/DKCustomFormFilter.h"

class HeadPartPickerFilter final : public DKCustomFormFilter {
   public:
      using head_part_type = dovah::head_part_type;

   public:
      using DKCustomFormFilter::DKCustomFormFilter;
         
      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

   public:
      void setRequiredRace(dovah::form_stub*);
      void setRequiredSex(dovah::sex);
      void setRequiredType(std::optional<head_part_type>);

   protected:
      struct {
         dovah::form_stub* race = nullptr;
         dovah::sex        sex  = dovah::sex::male;
         std::optional<head_part_type> type;
      } _params;

};