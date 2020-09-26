#pragma once
#include <QComboBox>
#include "../../dovah/core.h"

class FormSignatureCombobox : public QComboBox {
   Q_OBJECT
   public:
      FormSignatureCombobox(QWidget* parent = nullptr);
      //
      void whitelistAllSignatures();
      void whitelistSignature(uint32_t);
      QVector<uint32_t> whitelistedSignatures() const noexcept;
      //
      inline bool allowNone() const noexcept { return this->_allowNone; }
      void setAllowNone(bool) noexcept; // set whether a "NONE" option appears
      void setNoneLabel(const QString&) noexcept;
      //
      inline dovah::form_type_t formType() const noexcept { return this->currentData().toInt(); }
      //
   protected:
      QVector<uint32_t> _whitelist;
      bool    _allowNone = false;
      QString _noneLabel;
      //
      void _rebuild();
};
