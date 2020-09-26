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
      inline bool allowUnfiltered() const noexcept { return this->_allowUnfiltered; }
      void setAllowUnfiltered(bool) noexcept;
      void setUnfilteredLabel(const QString&) noexcept;
      //
      inline dovah::form_type_t formType() const noexcept { return this->currentData().toInt(); }
      //
   protected:
      QVector<uint32_t> _whitelist;
      bool    _allowUnfiltered = false;
      QString _unfilteredLabel;
      //
      void _rebuild();
};
