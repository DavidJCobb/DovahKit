#pragma once
#include "../../../dovah/core.h"
#include "ui_ref_picker_window.h"

namespace dovah {
   class form_stub;
}

class RefPickerWindow : public QDialog {
   Q_OBJECT
   public:
      RefPickerWindow(QWidget* parent);
      //
      inline dovah::form_stub* cell() const { return this->_cell; }
      inline dovah::form_stub* reference() const { return this->_reference; }
      void setCell(dovah::form_stub*);
      void setReference(dovah::form_stub*);
      void setShowSelectPlayerButton(bool);
      //
   public slots:
      //
   signals:
      void cellChanged(dovah::form_stub*); // fires if JUST the cell changes
      void referenceChanged(dovah::form_stub*); // fires if the reference changes; may also imply a cell-change
      //
   protected:
      Ui::RefPickerWindow ui;
      dovah::form_stub* _cell      = nullptr;
      dovah::form_stub* _reference = nullptr;
      //
      void _populateRefList();
      void _stringifyRef(const dovah::form_stub&, QString&);
};