#include "./DKWinTaskbarProgress.h"

DKWinTaskbarProgress::DKWinTaskbarProgress(QObject* parent) : QObject(parent) {
}

void DKWinTaskbarProgress::hide() {
   this->setVisible(false);
}
void DKWinTaskbarProgress::pause() {
   this->setPaused(true);
}
void DKWinTaskbarProgress::reset() {
   this->setValue(this->_data.minimum);
}
void DKWinTaskbarProgress::resume() {
   bool was_paused  = this->isPaused();
   bool was_stopped = this->isStopped();
   if (was_stopped) {
      this->_data.stopped = false;
      emit stoppedChanged(false);
   }
   if (was_paused) {
      this->_data.paused = false;
      emit pausedChanged(false);
   }
}
void DKWinTaskbarProgress::setMaximum(int v) {
   if (this->_data.maximum == v)
      return;
   this->setRange(std::min(this->_data.minimum, v), v);
}
void DKWinTaskbarProgress::setMinimum(int v) {
   if (this->_data.minimum == v)
      return;
   this->setRange(v, std::max(this->_data.maximum, v));
}
void DKWinTaskbarProgress::setPaused(bool v) {
   if (this->_data.paused == v)
      return;
   this->_data.paused = v;
   emit pausedChanged(v);
}
void DKWinTaskbarProgress::setRange(int minimum, int maximum) {
   const bool changed_min = minimum != this->_data.minimum;
   const bool changed_max = maximum != this->_data.maximum;
   if (!changed_min && !changed_max)
      return;

   this->_data.minimum = minimum;
   this->_data.maximum = std::max(minimum, maximum);
   if (this->_data.value < this->_data.minimum || this->_data.value > this->_data.maximum) {
      this->reset();
   }

   if (changed_min)
      emit minimumChanged(this->_data.minimum);
   if (changed_max)
      emit maximumChanged(this->_data.maximum);
}
void DKWinTaskbarProgress::setValue(int v) {
   if (this->_data.value == v)
      return;
   this->_data.value = v;
   emit valueChanged(v);
}
void DKWinTaskbarProgress::setVisible(bool v) {
   if (this->_data.visible == v)
      return;
   this->_data.visible = v;
   emit visibilityChanged(v);
}
void DKWinTaskbarProgress::show() {
   this->setVisible(true);
}
void DKWinTaskbarProgress::stop() {
   if (this->isStopped())
      return;
   this->_data.stopped = true;
   emit stoppedChanged(true);
}