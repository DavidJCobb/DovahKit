#pragma once
#include <vector>
#include <QDialog>
#include "ui_ObjectReferenceNewLinkedRefDialog.h" // generated
namespace dovah {
   class form_stub;
}
class DKFormPickerExcludeListedFormsFilter;

class ObjectReferenceNewLinkedRefDialog : public QDialog {
   Q_OBJECT;
   public:
      ObjectReferenceNewLinkedRefDialog(QWidget* parent = nullptr);
      
   protected:
      Ui::ObjectReferenceNewLinkedRefDialog ui;
      dovah::form_stub* disallowed_ref = nullptr;
      DKFormPickerExcludeListedFormsFilter* keyword_filter = nullptr;

   public:
      dovah::form_stub* keyword() const;
      dovah::form_stub* ref() const;

      void setDisallowedKeywords(std::vector<dovah::form_stub*>&&);
      void setDisallowedRef(dovah::form_stub&);
};