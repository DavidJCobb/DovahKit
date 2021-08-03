#include "DovahscriptDialog.h"

void DovahscriptDialog::setVisible(bool state) {
   this->_visible = state;
   QDialog::setVisible(state);
}