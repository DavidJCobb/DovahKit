#include "./DKCompactObjectReferencePicker.h"
#include <cassert>
#include <QGridLayout>
#include <QLabel>
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/forms/factories/hardcoded.h" // for PlayerRef form ID
   #include "dovah/form_stub.h"
   #include "editor/subsystems/papyrus/core.h"
   #include "editor/subsystems/worldedit/core.h"
   #include "editor/core.h"
   #include "helpers/qt/strings.h"

   #include "./widget-dialogs/DKCompactObjectReferencePickerDialog.h"
#endif

DKCompactObjectReferencePicker::DKCompactObjectReferencePicker(QWidget* parent) : QWidget(parent) {
   auto* layout = new QGridLayout(this);
   layout->setContentsMargins(0, 0, 0, 0);

   this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

   this->_button = new QPushButton(tr("NONE"), this);
   layout->addWidget(this->_button);

   #pragma region Tab order
   {
      this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->setFocusProxy(this->_button);
   }
   #pragma endregion

   #if !defined(QT_DESIGNER_LIB)
   QObject::connect(this->_button, &QPushButton::clicked, this, [this]() {
      auto* modal = new DKCompactObjectReferencePickerDialog(this);
      modal->setRequiredFormType(this->requiredFormType());
      modal->setRequiredScriptname(this->requiredScriptname());
      modal->setValue(this->ref());

      modal->setWindowModality(Qt::WindowModality::WindowModal);
      QObject::connect(modal, &QDialog::accepted, this, [this, modal]() {
         this->setRef(modal->value());
      });
      QObject::connect(modal, &QDialog::finished, modal, &QObject::deleteLater);
      modal->show();
   });
   {
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         this->setRef(nullptr);
      });
   }
   #endif
}

void DKCompactObjectReferencePicker::setPlaceholder(QString v) {
   this->state.placeholder = v;
   this->_update_text();
}

#if !defined(QT_DESIGNER_LIB)
   void DKCompactObjectReferencePicker::setRef(dovah::form_stub* stub) {
      auto* prior = this->ref();
      if (stub == prior)
         return;
      this->state.value = stub;
      this->_update_text();
      emit this->refChanged(stub);
   }
#endif

#if !defined(QT_DESIGNER_LIB)
   const std::string& DKCompactObjectReferencePicker::requiredScriptname() const {
      return this->state.required_scriptname;
   }
   void DKCompactObjectReferencePicker::setRequiredScriptname(QString desired) {
      this->setRequiredScriptname(desired.toUtf8().toStdString());
   }
   void DKCompactObjectReferencePicker::setRequiredScriptname(std::string_view desired) {
      this->state.required_scriptname = desired;
      if (desired.empty())
         return;
      auto* stub = this->ref();
      if (!stub)
         return;

      const auto& papyrus = dovahkit::subsystems::papyrus::core::get();
      if (!papyrus.form_has_script_attached(*stub, desired))
         this->setRef(nullptr);
   }
#endif

void DKCompactObjectReferencePicker::setRequiredFormType(dovah::form_type ft) {
   if (ft == dovah::form_type::none)
      ft = dovah::form_type::reference;
   if (ft == this->requiredFormType())
      return;
   #if !defined(QT_DESIGNER_LIB)
   #if _DEBUG
      if (!dovah::form_type_is_reference(ft)) {
         qWarning("DKCompactObjectReferencePicker is being told to require a form type that isn't REFR or a subclass; no forms will qualify");
      }
   #endif
   #endif
   this->state.required_form_type = ft;
   #if !defined(QT_DESIGNER_LIB)
      if (ft != dovah::form_type::reference) {
         auto* ref = this->ref();
         if (ref && ref->form_type != ft)
            this->setRef(nullptr);
      }
   #endif
}

void DKCompactObjectReferencePicker::_update_text() {
   #if defined(QT_DESIGNER_LIB)
      if (this->state.placeholder.isEmpty()) {
         this->_button->setText(tr("NONE"));
         return;
      }
      this->_button->setText(this->state.placeholder);
      return;
   #else
      auto* stub = this->ref();
      if (!stub) {
         this->_button->setText(tr("NONE"));
         return;
      }

      QString text = stub->get_editor_id();
      if (!text.isEmpty()) {
         this->_button->setText(text);
         return;
      }

      auto signature = cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(stub->form_type).signature);
      auto form_id   = QString("%1").arg(stub->formID, 8, 16, QChar('0')).toUpper();
      
      auto* cell = stub->get_parent_form();
      if (!cell || cell->form_type != dovah::form_type::cell) {
         this->_button->setText(tr("[%1:%2]").arg(signature).arg(form_id));
         return;
      }

      text = tr("[%1:%2] in %3").arg(signature).arg(form_id);
      //
      QString cell_text = cell->get_editor_id();
      if (cell_text.isEmpty()) {
         auto* world = cell->get_parent_form();
         if (world && world->form_type == dovah::form_type::worldspace) {
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
      text = text.arg(cell_text);
      //
      this->_button->setText(text);
   #endif
}