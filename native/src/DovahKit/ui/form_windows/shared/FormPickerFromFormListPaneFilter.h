#pragma once
#include <QPointer>
#include "widgets/widget-data/DKFormPickerCustomFilter.h"

class DKFormListPane;

//
// Filter the options in a DKFormPicker to only those forms that are 
// currently in a given DKFormListPane.
//
class FormPickerFromFormListPaneFilter final : public DKFormPickerCustomFilter {
   public:
      using DKFormPickerCustomFilter::DKFormPickerCustomFilter;
         
      virtual bool form_matches(const dovah::form_stub& stub) const noexcept override;

   public:
      DKFormListPane* pane() const;
      void setPane(DKFormListPane*);

   protected:
      QPointer<DKFormListPane> _pane;

      void _set_pane_impl(DKFormListPane*);

};