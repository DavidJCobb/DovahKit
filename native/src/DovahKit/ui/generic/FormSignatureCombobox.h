#pragma once
#include <cstdint>
#include <QComboBox>
#include "dovah/form_types.h"

class FormSignatureCombobox : public QComboBox {
   Q_OBJECT
   public:
      FormSignatureCombobox(QWidget* parent = nullptr);
      //
      void whitelistAllSignatures();
      void whitelistSignature(uint32_t);
      QVector<uint32_t> whitelistedSignatures() const noexcept;
      //
      inline bool allowUnfiltered() const noexcept { return this->_allowUnfiltered; }
      void setAllowUnfiltered(bool) noexcept;
      void setUnfilteredLabel(const QString&) noexcept;
      //
      inline dovah::form_type formType() const noexcept { return (dovah::form_type)this->currentData().toInt(); }
      //
   protected:
      QVector<uint32_t> _whitelist;
      bool    _allowUnfiltered = false;
      QString _unfilteredLabel;
      //
      void _rebuild();
};
