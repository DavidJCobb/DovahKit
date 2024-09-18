#pragma once
#include <QPointer>
#include "widgets/widget-data/DKCustomFormFilter.h"

class DKFormListPane;

//
// Filter the options in a DKFormPicker to only those forms that are 
// currently in a given DKFormListPane.
//
class FormPickerFromFormListPaneFilter final : public DKCustomFormFilter {
   public:
      using DKCustomFormFilter::DKCustomFormFilter;
         
      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

   public:
      DKFormListPane* pane() const;
      void setPane(DKFormListPane*);

   protected:
      QPointer<DKFormListPane> _pane;

      void _set_pane_impl(DKFormListPane*);

};