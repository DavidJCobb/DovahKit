#include "QItemSelectionModelEx.h"

QItemSelectionModelEx::QItemSelectionModelEx(QAbstractItemModel* model, QObject* parent) : QItemSelectionModel(model, parent) {
   this->_init();
}
QItemSelectionModelEx::QItemSelectionModelEx(QAbstractItemModel* model) : QItemSelectionModel(model) {
   this->_init();
}

//
// As of this writing, QItemSelectionModel directly modifies its selection state 
// when reacting to the imminent removal of table rows/columns. In all other cases 
// (i.e. when QAbstractItemView and subclasses modify the selection in response to 
// user interactions), the virtual `select` member function is called.
// 
// The `select` function has two overloads. The one that takes a QModelIndex just 
// creates a new selection beginning and ending at that index, before forwarding 
// that to the other overload. As such, we only need to override the overload that 
// takes a QItemSelection.
//

void QItemSelectionModelEx::_init() {
   QObject::connect(this, &QItemSelectionModel::selectionChanged, this, &QItemSelectionModelEx::_onSelectionChanged);
}
void QItemSelectionModelEx::_onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
   if (!this->_user_initiated)
      return;
   this->_user_initiated = false;
   emit this->userInitiatedSelectionChanged(selected, deselected);
}
void QItemSelectionModelEx::select(const QItemSelection& s, SelectionFlags f) {
   this->_user_initiated = true;
   QItemSelectionModel::select(s, f);
   this->_user_initiated = false;
}