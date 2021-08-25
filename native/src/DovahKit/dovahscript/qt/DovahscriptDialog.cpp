#include "DovahscriptDialog.h"

void DovahscriptDialog::setVisible(bool state) {
   this->_visible = state;
   QDialog::setVisible(state);
   if (state)
      emit shown();
   else
      emit hidden();
}