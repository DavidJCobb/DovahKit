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
      void setAllowNone(bool) noexcept;
      void setFormByID(dovah::bare_form_id_t) noexcept;
      //
      static void populate(dovah::form_type_t, QVector<FormsOfTypeCombobox*>&);
      //
   protected:
      bool _allowNone = false;
      QVector<dovah::form_type_t> _formTypes;
      //
   signals:
      void populated();
};
