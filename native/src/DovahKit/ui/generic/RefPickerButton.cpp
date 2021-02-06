#include "RefPickerButton.h"
#include "../../dovah/form_stub.h"
#include "../../helpers/qt/strings.h"
#include "../../editor/core.h"
#include "windows/ref_picker_window.h"

RefPickerButton::RefPickerButton(QWidget* parent) : QPushButton(parent) {
   QObject::connect(this, &QPushButton::clicked, this, [this]() {
      auto* window = new RefPickerWindow(this);
      window->setReference(this->_stub);
      QObject::connect(window, &QDialog::accepted, this, [this, window]() {
         this->setValue(window->reference());
      });
      window->open();
   });
   this->_placeholder = tr("NONE", "ref picker placeholder");
   this->setText(this->_placeholder);
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool will_be_flagged) {
      if (stub == this->_stub)
         this->setValue(nullptr);
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      if (stub == this->_stub)
         this->_updateText();
   });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->setValue(nullptr);
   });
}
void RefPickerButton::setValue(dovah::form_stub* stub) {
   if (this->_stub == stub)
      return;
   this->_stub = stub;
   this->_updateText();
   emit this->valueChanged(stub);
}
void RefPickerButton::_updateText() {
   if (auto* stub = this->_stub) {
      QString text = stub->get_editor_id();
      if (text.isEmpty()) {
         text = tr("[%1:%2] in %3")
            .arg(cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(stub->formType).signature))
            .arg(QString("%1").arg(stub->formID, 8, 16, QChar('0')).toUpper());
         //
         QString cell_text;
         auto& editor = DovahKitCore::get();
         auto* cell   = editor.get_form(stub->parentID);
         if (cell) {
            cell_text = cell->get_editor_id();
            if (cell_text.isEmpty() && cell->parentID) {
               auto* world = editor.get_form(cell->parentID);
               if (world) {
                  int32_t x;
                  int32_t y;
                  cell_text = tr("(%1, %2) in %3");
                  if (cell->get_grid_coordinates(x, y)) {
                     cell_text = cell_text.arg(x).arg(y);
                  } else {
                     cell_text = cell_text.arg("?").arg("?");
                  }
                  cell_text = cell_text.arg(world->get_editor_id());
               }
            }
         } else {
            cell_text = tr("NONE", "ref picker - no parent cell");
         }
         text = text.arg(cell_text);
      }
      this->setText(text);
   } else {
      this->setText(this->_placeholder);
   }
}