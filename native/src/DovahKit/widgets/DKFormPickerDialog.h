#pragma once
#if defined(QT_DESIGNER_LIB)
   #error This widget is not meant to be usable in Qt Designer.
#endif
#include <QAbstractItemModel>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include "dovah/form_types.h"

namespace dovah {
   class form_stub;
}
namespace ui::impl::DKFormPicker {
   class DialogModel;
}

class DKFormPickerDialog : public QDialog {
   Q_OBJECT;
   Q_PROPERTY(QList<dovah::form_type> allowedFormTypes READ allowedFormTypes WRITE setAllowedFormTypes DESIGNABLE true USER true);
   public:
      DKFormPickerDialog(QWidget* parent = nullptr);
      
      constexpr const QList<dovah::form_type>& allowedFormTypes() const noexcept { return this->_properties.allowed_form_types; }
      void setAllowedFormTypes(QList<dovah::form_type>) noexcept;
      //
      void addAllowedFormType(dovah::form_type);
      inline void allowAllFormTypes() noexcept { this->setAllowedFormTypes({}); }
      inline void setAllowedFormType(dovah::form_type ft) noexcept { this->setAllowedFormTypes({ ft }); }
      //
      inline bool allowsFormType(dovah::form_type ft) const noexcept {
         return this->_properties.allowed_form_types.contains(ft);
      }

      #if !defined(QT_DESIGNER_LIB)
         constexpr dovah::form_stub* formStub() const noexcept { return this->_value; }
         void setFormStub(dovah::form_stub*) noexcept;
      #endif
         
   protected:
      #if !defined(QT_DESIGNER_LIB)
         dovah::form_stub* _value = nullptr;
      #endif
      ui::impl::DKFormPicker::DialogModel* _model = nullptr;
      struct {
         QList<dovah::form_type> allowed_form_types;
      } _properties;
      struct {
         QLineEdit*  filter = nullptr;
         QTableView* table  = nullptr;
      } _subwidgets;

      virtual void changeEvent(QEvent* event) override;
};