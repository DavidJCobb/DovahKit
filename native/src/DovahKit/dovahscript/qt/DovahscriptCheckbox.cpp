#include "DovahscriptCheckbox.h"

void DovahscriptCheckbox::nextCheckState() {
   if (!this->isCheckable())
      return;
   if (this->checkState() == Qt::Checked)
      this->setCheckState(Qt::Unchecked);
   else
      this->setCheckState(Qt::Checked);
}