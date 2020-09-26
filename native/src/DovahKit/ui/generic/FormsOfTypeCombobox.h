#pragma once
#include <QComboBox>
#include "../../dovah/core.h"

class FormsOfTypeCombobox : public QComboBox {
   Q_OBJECT
   public:
      FormsOfTypeCombobox(QWidget* parent = nullptr);
      //
      void addFormType(dovah::form_type_t);
      inline bool allowNone() const noexcept { return this->_allowNone; }
      bool allowsFormType(dovah::form_type_t) const noexcept;
      dovah::bare_form_id_t formID() const noexcept;
      void populate();
      //
      void setAllowNone(bool) noexcept; // set whether a "NONE" option appears
      void setNoneLabel(const QString&) noexcept;
      //
      void setFormByID(dovah::bare_form_id_t) noexcept; // set value
      void setDefaultFormID(dovah::bare_form_id_t) noexcept; // used if you call setFormByID(0) and allow-none is false
      //
      static void populate(dovah::form_type_t, QVector<FormsOfTypeCombobox*>&);
      //
   protected:
      bool _allowNone = false;
      QString _noneLabel;
      QVector<dovah::form_type_t> _formTypes;
      dovah::bare_form_id_t _defaultFormID = 0;
      //
   signals:
      void populated();
};
