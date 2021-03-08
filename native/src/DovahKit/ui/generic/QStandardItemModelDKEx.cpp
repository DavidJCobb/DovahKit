#include "QStandardItemModelDKEx.h"

void QStandardItemModelDKEx::_forceUpdateAll(const QVector<int>& roles) {
   auto rc = this->rowCount();
   if (!rc)
      return;
   QModelIndex tl = this->index(0, 0);
   QModelIndex br = this->index(rc, this->columnCount());
   emit dataChanged(tl, br, roles);
}

void QStandardItemModelDKEx::setAutoTooltips(bool v) {
   if (this->_auto_tooltip == v)
      return;
   this->_auto_tooltip = v;
   this->_forceUpdateAll({ Qt::ToolTipRole });
}
QVariant QStandardItemModelDKEx::data(const QModelIndex& index, int role) const {
   if (role == Qt::ToolTipRole)
      role = Qt::DisplayRole;
   return QStandardItemModel::data(index, role);
};