#pragma once
#include <QComboBox>
#include <QStandardItemModel>
#include <map>
#include "../../dovah/core.h"

namespace FormPickerImpl {
   class FormPickerProxyModel;
}

class FormPicker : public QWidget {
   Q_OBJECT
   public:
      FormPicker(QWidget* parent = nullptr);
      
      inline const QVector<dovah::form_type_t>& allowedFormTypes() const noexcept { return this->_formTypes; }
      inline bool allowNone() const noexcept { return this->_allowNone; }
      inline bool splitTypesWhenMany() const noexcept { return this->_splitTypesWhenMany; }

      inline bool allowsFormType(dovah::form_type_t ft) const noexcept {
         return this->_formTypes.contains(ft);
      }
      inline bool isSplittingTypes() const noexcept {
         return this->subwidgets.type->isVisible();
      }

      dovah::bare_form_id_t formID() const noexcept;
      dovah::form_stub* formStub() const noexcept;

      void addFormType(dovah::form_type_t);
      inline void allowAllFormTypes() noexcept { this->setAllowedFormTypes({}); }
      inline void setAllowedFormType(dovah::form_type_t ft) noexcept { this->setAllowedFormTypes({ ft }); }
      void setAllowedFormTypes(QVector<dovah::form_type_t>) noexcept;
      void setSplitTypesWhenMany(bool) noexcept;
      
      void setAllowNone(bool) noexcept; // set whether a "NONE" option appears
      
      void setFormByID(dovah::bare_form_id_t) noexcept;
      void setFormStub(dovah::form_stub*) noexcept;
      void setDefaultFormID(dovah::bare_form_id_t) noexcept; // used if you call setFormByID(0) and allow-none is false
      
   protected:
      struct _last_selection {
         dovah::form_type_t    type;
         dovah::bare_form_id_t formID = 0;
      };

      QVector<dovah::form_type_t> _formTypes;
      bool    _activated = false;
      bool    _allowNone = false;
      bool    _splitTypesWhenMany = true;
      struct {
         QComboBox* form = nullptr;
         QComboBox* type = nullptr;
      } subwidgets;
      std::map<dovah::form_type_t, dovah::bare_form_id_t> _prior_selections;

      virtual void changeEvent(QEvent* event) override;
      virtual void showEvent(QShowEvent* event) override;

      FormPickerImpl::FormPickerProxyModel* _rawModel() const noexcept;

      void _activate();
      void _setIsSplittingTypes(bool) noexcept;
      bool _shouldSplitTypes() const noexcept;
      void _updateForms();
      void _updateTypePicker();
      
   signals:
      void formChanged(dovah::form_stub*);
      void populated();
};