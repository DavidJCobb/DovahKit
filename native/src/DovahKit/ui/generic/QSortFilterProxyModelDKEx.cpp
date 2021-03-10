#include "QSortFilterProxyModelDKEx.h"

void QSortFilterProxyModelDKEx::setLessThanFunction(less_than_handler_t functor) {
   this->_functors.lessThan = functor;
   this->invalidate();
}
void QSortFilterProxyModelDKEx::setFilterAcceptsColumnFunction(filter_accept_handler_t functor) {
   this->_functors.filterAcceptsColumn = functor;
   this->invalidateFilter();
}
void QSortFilterProxyModelDKEx::setFilterAcceptsRowFunction(filter_accept_handler_t functor) {
   this->_functors.filterAcceptsRow = functor;
   this->invalidateFilter();
}

bool QSortFilterProxyModelDKEx::defaultLessThanFunction(const QModelIndex& a, const QModelIndex& b) const {
   return QSortFilterProxyModel::lessThan(a, b);
}
bool QSortFilterProxyModelDKEx::defaultFilterAcceptsColumn(int col, const QModelIndex& parent) const {
   return QSortFilterProxyModel::filterAcceptsColumn(col, parent);
}
bool QSortFilterProxyModelDKEx::defaultFilterAcceptsRow(int row, const QModelIndex& parent) const {
   return QSortFilterProxyModel::filterAcceptsRow(row, parent);
}

bool QSortFilterProxyModelDKEx::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const {
   if (auto functor = this->_functors.lessThan)
      return (functor)(*this, source_left, source_right);
   return this->defaultLessThanFunction(source_left, source_right);
}
bool QSortFilterProxyModelDKEx::filterAcceptsColumn(int col, const QModelIndex& parent) const {
   if (auto functor = this->_functors.filterAcceptsColumn)
      return (functor)(*this, col, parent);
   return this->defaultFilterAcceptsColumn(col, parent);
}
bool QSortFilterProxyModelDKEx::filterAcceptsRow(int row, const QModelIndex& parent) const {
   if (auto functor = this->_functors.filterAcceptsRow)
      return (functor)(*this, row, parent);
   return this->defaultFilterAcceptsRow(row, parent);
}