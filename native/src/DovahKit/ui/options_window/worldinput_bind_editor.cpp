#include "./worldinput_bind_editor.h"

WorldinputBindEditDialog::WorldinputBindEditDialog(QWidget* parent) : QDialog(parent) {
   QObject::connect(this->ui.raycastReqTargetGizmoEnable, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.raycastReqTargetGizmoAxis->setEnabled(checked);
      this->ui.raycastReqTargetGizmoMode->setEnabled(checked);
   });
}