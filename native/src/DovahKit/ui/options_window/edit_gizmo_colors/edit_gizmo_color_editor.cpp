#include "./edit_gizmo_color_editor.h"

namespace {
   void _qt_to_mine(const QColor& src, cobb::color::rgb_bytes& dst) {
      dst.components = { (uint8_t)src.red(), (uint8_t)src.green(), (uint8_t)src.blue() };
   }
}

EditGizmoColorSchemeEditDialog::EditGizmoColorSchemeEditDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonSave, &QPushButton::clicked, this, &QDialog::accept);
}
void EditGizmoColorSchemeEditDialog::initializeFrom(const gizmo_color_scheme& src) {
   this->ui.name->setText(QString::fromUtf8(src.name.c_str(), src.name.size()));
   this->ui.colorAxisX->setColor(QColor(
      src.axis_x.r,
      src.axis_x.g,
      src.axis_x.b
   ));
   this->ui.colorAxisY->setColor(QColor(
      src.axis_y.r,
      src.axis_y.g,
      src.axis_y.b
   ));
   this->ui.colorAxisZ->setColor(QColor(
      src.axis_z.r,
      src.axis_z.g,
      src.axis_z.b
   ));
   this->ui.colorHighlight->setColor(QColor(
      src.highlight.r,
      src.highlight.g,
      src.highlight.b
   ));
}
void EditGizmoColorSchemeEditDialog::overwrite(gizmo_color_scheme& dst) const {
   dst.name = this->ui.name->text().toUtf8().toStdString();
   _qt_to_mine(this->ui.colorAxisX->color(), dst.axis_x);
   _qt_to_mine(this->ui.colorAxisY->color(), dst.axis_y);
   _qt_to_mine(this->ui.colorAxisZ->color(), dst.axis_z);
   _qt_to_mine(this->ui.colorHighlight->color(), dst.highlight);
}