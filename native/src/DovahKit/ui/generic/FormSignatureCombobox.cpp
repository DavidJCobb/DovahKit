#include "FormSignatureCombobox.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include "../../helpers/qt/strings.h"
#include "../../dovah/core.h"

FormSignatureCombobox::FormSignatureCombobox(QWidget* parent) : QComboBox(parent) {
   this->_rebuild();
};

void FormSignatureCombobox::whitelistAllSignatures() {
   this->_whitelist.clear();
   this->_rebuild();
}
void FormSignatureCombobox::whitelistSignature(uint32_t signature) {
   if (signature == dovah::form_types[dovah::form_type::none].signature)
      return;
   if (this->_whitelist.contains(signature))
      return;
   this->_whitelist.push_back(signature);
   this->_rebuild();
}
QVector<uint32_t> FormSignatureCombobox::whitelistedSignatures() const noexcept {
   return this->_whitelist;
}

void FormSignatureCombobox::_rebuild() {
   const auto blocker = QSignalBlocker(this);
   auto previous_selection = this->currentData().toInt();
   this->clear();
   //
   auto* model        = new QStandardItemModel(this);
   bool  whitelisting = this->_whitelist.size();
   for (auto& info : dovah::form_types) {
      if (whitelisting && !this->_whitelist.contains(info.signature))
         continue;
      switch (info.formType) {
         case dovah::form_type::none:
         case dovah::form_type::file_header:
         case dovah::form_type::file_record_group:
         case dovah::form_type::setting:
            continue;
      }
      auto* item = new QStandardItem(cobb::qt::four_cc_to_string(info.signature));
      item->setData(info.formType, Qt::UserRole);
      model->appendRow(item);
   }
   if (this->_allowNone) {
      QString text = this->_noneLabel;
      if (text.isEmpty())
         text = tr("NONE");
      auto* item = new QStandardItem(text);
      item->setData(dovah::form_type::none, Qt::UserRole);
      model->appendRow(item);
   }
   //
   auto* proxy = new QSortFilterProxyModel;
   proxy->setSourceModel(model);
   proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
   this->setModel(proxy);
   proxy->sort(0, Qt::AscendingOrder);
   //
   int index = this->findData(previous_selection, Qt::UserRole);
   if (index >= 0)
      this->setCurrentIndex(index);
   else {
      index = -1;
      if (this->_allowNone)
         index = this->findData(dovah::form_type::none, Qt::UserRole);
      this->setCurrentIndex(index);
   }
}
void FormSignatureCombobox::setAllowNone(bool s) noexcept {
   if (this->_allowNone == s)
      return;
   this->_allowNone = s;
   if (s) {
      QString text = this->_noneLabel;
      if (text.isEmpty())
         text = tr("NONE");
      this->addItem(text, 0);
   } else {
      int i = this->findData(0);
      if (i >= 0)
         this->removeItem(i);
   }
}
void FormSignatureCombobox::setNoneLabel(const QString& v) noexcept {
   this->_noneLabel = v;
   if (this->_allowNone) {
      int index = this->findData(0, Qt::UserRole);
      if (index < 0)
         return;
      this->setItemText(index, this->_noneLabel);
   }
}