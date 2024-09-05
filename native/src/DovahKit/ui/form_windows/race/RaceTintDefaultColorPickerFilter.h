#pragma once
#include <QPointer>
#include "widgets/widget-data/DKFormPickerCustomFilter.h"
#include "./RaceTintLayerModel.h"

class RaceTintDefaultColorPickerFilter final : public DKFormPickerCustomFilter {
   public:
      using DKFormPickerCustomFilter::DKFormPickerCustomFilter;

      virtual bool form_matches(const dovah::form_stub& stub) const noexcept override;

   public:
      void setModel(RaceTintLayerModel*);
      void setLayer(const QModelIndex&);

   protected:
      QPointer<RaceTintLayerModel> _model;
      QPersistentModelIndex _layer;

      void _on_model_structure_change(const QModelIndex& parent);
      void _on_model_data_change(const QModelIndex& subject);
};