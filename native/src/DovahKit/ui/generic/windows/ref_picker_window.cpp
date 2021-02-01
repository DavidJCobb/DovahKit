#include "ref_picker_window.h"
#include <QMessageBox>
#include "../../../dovah/core.h"
#include "../../../dovah/form_stub.h"
#include "../../../dovah/forms/factories/hardcoded.h"
#include "../../../editor/core.h"

RefPickerWindow::RefPickerWindow(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->_cell      = nullptr;
      this->_reference = nullptr;
      this->reject();
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool will_be_flagged) {
      if (stub->formType == dovah::form_type::cell) {
         auto* widget  = this->ui.cell;
         auto  blocker = const QSignalBlocker(widget);
         //
         auto i = widget->findData(QVariant::fromValue<void*>(stub));
         if (i >= 0)
            widget->removeItem(i);
         //
         if (this->_cell == stub) {
            this->_cell      = nullptr;
            this->_reference = nullptr;
            emit this->cellChanged(nullptr);
         }
         return;
      }
      if (dovah::form_type_info::form_type_is_reference(stub->formType)) {
         if (!this->_cell)
            return;
         if (stub->groupInfo.parentFormID == this->_cell->formID) {
            auto* widget  = this->ui.ref;
            auto  blocker = const QSignalBlocker(widget);
            //
            auto i = widget->findData(QVariant::fromValue<void*>(stub));
            if (i >= 0)
               widget->removeItem(i);
         }
         if (stub == this->_reference) {
            this->_reference = nullptr;
            emit this->referenceChanged(nullptr);
         }
      }
   });
   //
   QObject::connect(this->ui.buttonRenderPick, &QPushButton::clicked, this, [this]() {
      //
      // TODO: Let the user pick a reference from the render window.
      //
      QMessageBox::critical(this, tr("Error"), tr("Not implemented"));
   });
   QObject::connect(this->ui.cell, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
      auto* change_to = (dovah::form_stub*) this->ui.cell->currentData().value<void*>();
      this->setCell(change_to);
   });
   QObject::connect(this->ui.ref, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
      auto* change_to = (dovah::form_stub*) this->ui.ref->currentData().value<void*>();
      this->setReference(change_to);
   });
   QObject::connect(this->ui.buttonRenderView, &QPushButton::clicked, this, [this]() {
      //
      // TODO: Show the currently-selected reference in the render window.
      //
      QMessageBox::critical(this, tr("Error"), tr("Not implemented"));
   });
   QObject::connect(this->ui.buttonSelectPlayer, &QPushButton::clicked, this, [this]() {
      auto& editor = DovahKitCore::get();
      auto* form   = editor.get_form(dovah::hardcoded_form_ids::PlayerRef);
      this->setReference(form);
   });
   //
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, [this]() {
      this->accept();
   });
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, [this]() {
      this->reject();
   });
}

void RefPickerWindow::setCell(dovah::form_stub* stub) {
   if (stub == this->_cell)
      return;
   this->_cell      = stub;
   this->_reference = nullptr;
   this->_populateRefList();
   emit this->cellChanged(stub);
}
void RefPickerWindow::setReference(dovah::form_stub* stub) {
   if (stub && stub->is_none_stub())
      stub = nullptr;
   if (stub == this->_reference)
      return;
   if (!stub) {
      this->_reference = nullptr;
      emit this->referenceChanged(nullptr);
      return;
   }
   dovah::bare_form_id_t parentID = (this->_cell) ? this->_cell->formID : 0;
   if (stub->groupInfo.parentFormID != parentID) {
      auto& editor = DovahKitCore::get();
      auto* parent = editor.get_form(parentID);
      this->_cell = parent;
      this->_populateRefList();
   }
   this->_reference = stub;
   //
   auto* widget  = this->ui.ref;
   auto  blocker = const QSignalBlocker(widget);
   widget->setCurrentIndex(widget->findData(QVariant::fromValue<void*>(stub)));
   //
   emit this->referenceChanged(stub);
}
void RefPickerWindow::setShowSelectPlayerButton(bool s) {
   this->ui.buttonSelectPlayer->setVisible(s);
}

void RefPickerWindow::_populateRefList() {
   auto* widget  = this->ui.ref;
   auto  blocker = const QSignalBlocker(widget);
   widget->clear();
   widget->addItem(tr("NONE"), QVariant::fromValue<void*>(nullptr));
   widget->setCurrentIndex(0);
   widget->setEnabled(true);
   //
   if (this->_cell) {
      for (auto& pair : this->_cell->inbound) {
         auto& entry = pair.second;
         if (entry.flags & dovah::use_info_entry::flag::i_am_parent_of) {
            auto* other = entry.other;
            widget->addItem(other ? other->get_editor_id() : "", QVariant::fromValue<void*>(other));
         }
      }
   } else {
      auto& editor = DovahKitCore::get();
      auto* stub   = editor.get_form(dovah::hardcoded_form_ids::PlayerRef);
      if (stub) {
         widget->setEnabled(false);
      } else {
         widget->addItem(stub->get_editor_id(), QVariant::fromValue<void*>(stub));
      }
   }
}