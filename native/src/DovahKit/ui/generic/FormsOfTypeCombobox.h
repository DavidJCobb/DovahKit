#pragma once
#include <QComboBox>
#include "../../dovah/core.h"

class FormsOfTypeCombobox : public QComboBox {
   Q_OBJECT
   protected:
      static constexpr Qt::ItemDataRole FormIDRole    = (Qt::ItemDataRole)Qt::UserRole;
      static constexpr Qt::ItemDataRole FormStubRole  = (Qt::ItemDataRole)(Qt::UserRole + 1);
      static constexpr Qt::ItemDataRole UndefinedRole = (Qt::ItemDataRole)(Qt::UserRole + 2);
      //
   public:
      FormsOfTypeCombobox(QWidget* parent = nullptr);
      //
      void addFormType(dovah::form_type_t);
      inline bool allowNone() const noexcept { return this->_allowNone; }
      bool allowsFormType(dovah::form_type_t) const noexcept;
      dovah::bare_form_id_t formID() const noexcept;
      dovah::form_stub* formStub() const noexcept;
      void populate();
      //
      void setAllowNone(bool) noexcept; // set whether a "NONE" option appears
      void setNoneLabel(const QString&) noexcept;
      //
      void setAllowUndefined(bool) noexcept; // set whether an "UNDEFINED" option appears
      void setUndefinedLabel(const QString&) noexcept;
      bool isUndefined() const noexcept;
      void setToUndefined() noexcept; // fails if undefined is not allowed
      //
      void setFormByID(dovah::bare_form_id_t) noexcept; // set value
      void setDefaultFormID(dovah::bare_form_id_t) noexcept; // used if you call setFormByID(0) and allow-none is false
      //
   protected:
      bool    _allowNone = false;
      QString _noneLabel;
      bool    _allowUndefined = false;
      QString _undefinedLabel;
      QVector<dovah::form_type_t> _formTypes;
      dovah::bare_form_id_t _defaultFormID = 0;
      //
   signals:
      void populated();
};
