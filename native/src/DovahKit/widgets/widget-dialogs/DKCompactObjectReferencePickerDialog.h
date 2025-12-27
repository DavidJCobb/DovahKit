#pragma once
#include "ui_DKCompactObjectReferencePickerDialog.h" // generated
#include <functional>
#include <string>
#include <QDialog>

namespace dovah {
   class form_stub;
}

class DKCompactObjectReferencePickerDialog : public QDialog {
   Q_OBJECT;
   public:
      DKCompactObjectReferencePickerDialog(QWidget* parent = nullptr);

      dovah::form_stub* value() const;
      void setValue(dovah::form_stub*);

      dovah::form_type requiredFormType() const;
      void setRequiredFormType(dovah::form_type);

      const std::string& requiredScriptname() const;
      void setRequiredScriptname(QString);
      void setRequiredScriptname(std::string_view);

      void setValidationFunction(const std::function<bool(dovah::form_stub*)>&);

   protected:
      Ui::DKCompactObjectReferencePickerDialog ui;
      struct {
         dovah::form_stub* value = nullptr;
      } state;
};