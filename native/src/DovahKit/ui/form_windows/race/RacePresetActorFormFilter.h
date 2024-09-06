#pragma once
#include <QPointer>
#include "widgets/widget-data/DKFormListPaneCustomFilter.h"
#include "./RaceTintLayerModel.h"

class RacePresetActorFormFilter final : public DKFormListPaneCustomFilter {
   public:
      using DKFormListPaneCustomFilter::DKFormListPaneCustomFilter;

      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

   public:
      void setRace(dovah::form_stub*);
      void setSex(dovah::sex);

   protected:
      dovah::form_stub* race = nullptr;
      dovah::sex        sex  = dovah::sex::female;
};