#pragma once
#include <QCheckBox>

class DovahscriptCheckbox : public QCheckBox {
   Q_OBJECT;
   public:
      using QCheckBox::QCheckBox;
   protected:
      //
      // Checkboxes can be set to an "indeterminate" state (or, as Qt calls it, "partially checked") if 
      // they are marked as "tri-state." However, user interactions with a tri-state checkbox will cause 
      // it to cycle between its three possible states. We want scripts to be able to set a checkbox to 
      // "indeterminate" without changing user interaction, such that the user interaction always checks 
      // or unchecks a box. Simply avoiding QCheckBox::setTristate won't work, because setting the state 
      // to "indeterminate" automatically makes the checkbox tri-state.
      //
      virtual void nextCheckState() override;
};