#include "./DKScopedProxyModel.h"

std::optional<QModelIndex> DKScopedProxyModel::scopeIndex() const {
   return this->_scope.index;
}
void DKScopedProxyModel::setScopeIndex(const std::optional<QModelIndex>& opt_source_qmi) {
   if (!opt_source_qmi.has_value()) {
      if (!this->_scope.index.has_value())
         return;
      this->beginResetModel();
      this->_scope.index = opt_source_qmi;
      this->endResetModel();
      return;
   }
   const auto& source_qmi = opt_source_qmi.value();
   assert(source_qmi.model() == this->sourceModel());
   if (this->_scope.index == source_qmi)
      return;
   this->beginResetModel();
   this->_scope.index = source_qmi;
   this->endResetModel();
}

bool DKScopedProxyModel::scopeVisible() const {
   return this->_scope.visible;
}
void DKScopedProxyModel::setScopeVisible(bool v) {
   if (v == this->_scope.visible)
      return;
   if (!this->_scope.index.has_value()) {
      this->_scope.visible = v;
      return;
   }
   this->beginResetModel();
   this->_scope.visible = v;
   this->endResetModel();
}

#pragma region QAbstractItemModel overrides
   /*virtual*/ Qt::ItemFlags DKScopedProxyModel::flags(const QModelIndex& qmi) const /*override*/ {
      if (_proxy_qmi_is_scope_parent(qmi)) {
         return Qt::ItemFlag::ItemIsEnabled;
      }
      return QIdentityProxyModel::flags(qmi);
   }
   /*virtual*/ QModelIndex DKScopedProxyModel::index(int row, int column, const QModelIndex& proxy_parent_qmi) const /*override*/ {
      if (!this->_scope.index.has_value())
         return {};
      const auto& scope = this->_scope.index.value();
      if (_proxy_qmi_is_scope_parent(proxy_parent_qmi)) {
         if (row != 0 || column != 0)
            return {};
         return this->mapFromSource(scope);
      }
      return QIdentityProxyModel::index(row, column, proxy_parent_qmi);
   }
   /*virtual*/ int DKScopedProxyModel::columnCount(const QModelIndex& proxy_qmi) const /*override*/ {
      const auto* source_model = this->sourceModel();
      if (!source_model)
         return 0;
      if (!this->_scope.index.has_value())
         return 0;
      if (_proxy_qmi_is_scope_parent(proxy_qmi))
         return 1;
      if (_proxy_qmi_is_scope(proxy_qmi))
         return source_model->columnCount(this->_scope.index.value());
      return QIdentityProxyModel::columnCount(proxy_qmi);
   }
   /*virtual*/ int DKScopedProxyModel::rowCount(const QModelIndex& proxy_qmi) const /*override*/ {
      const auto* source_model = this->sourceModel();
      if (!source_model)
         return 0;
      if (!this->_scope.index.has_value())
         return 0;
      if (_proxy_qmi_is_scope_parent(proxy_qmi))
         return 1;
      if (_proxy_qmi_is_scope(proxy_qmi))
         return source_model->rowCount(this->_scope.index.value());
      return QIdentityProxyModel::rowCount(proxy_qmi);
   }
   /*virtual*/ QModelIndex DKScopedProxyModel::parent(const QModelIndex& proxy_qmi) const /*override*/ {
      if (!proxy_qmi.isValid())
         return {};
      if (_proxy_qmi_is_scope_parent(proxy_qmi))
         return {};
      return QIdentityProxyModel::parent(proxy_qmi);
   }
   /*virtual*/ QModelIndex DKScopedProxyModel::sibling(int row, int col, const QModelIndex& proxy_qmi) const /*override*/ {
      if (!proxy_qmi.isValid())
         return {};
      if (this->_scope.visible) {
         if (_proxy_qmi_is_scope(proxy_qmi)) {
            if (row == 0 && col == 0)
               return proxy_qmi;
            return {};
         }
      }
      return QIdentityProxyModel::sibling(row, col, proxy_qmi);
   }
   #pragma region Modify hierarchy
      /*virtual*/ bool DKScopedProxyModel::dropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) /*override*/ {
         if (_proxy_qmi_is_scope_parent(parent))
            return false;
         return QIdentityProxyModel::dropMimeData(mime, action, row, column, parent);
      }
      /*virtual*/ bool DKScopedProxyModel::insertColumns(int i, int count, const QModelIndex& proxy_qmi) /*override*/ {
         if (_proxy_qmi_is_scope_parent(proxy_qmi))
            return false;
         return QIdentityProxyModel::insertColumns(i, count, proxy_qmi);
      }
      /*virtual*/ bool DKScopedProxyModel::insertRows(int i, int count, const QModelIndex& proxy_qmi) /*override*/ {
         if (_proxy_qmi_is_scope_parent(proxy_qmi))
            return false;
         return QIdentityProxyModel::insertRows(i, count, proxy_qmi);
      }
      /*virtual*/ bool DKScopedProxyModel::moveColumns(const QModelIndex& from_parent, int first, int last, const QModelIndex& to_parent, int to) /*override*/ {
         if (_proxy_qmi_is_scope_parent(from_parent))
            return false;
         if (_proxy_qmi_is_scope_parent(to_parent))
            return false;
         return QIdentityProxyModel::moveColumns(from_parent, first, last, to_parent, to);
      }
      /*virtual*/ bool DKScopedProxyModel::moveRows(const QModelIndex& from_parent, int first, int last, const QModelIndex& to_parent, int to) /*override*/ {
         if (_proxy_qmi_is_scope_parent(from_parent))
            return false;
         if (_proxy_qmi_is_scope_parent(to_parent))
            return false;
         return QIdentityProxyModel::moveColumns(from_parent, first, last, to_parent, to);
      }
      /*virtual*/ bool DKScopedProxyModel::removeColumns(int i, int count, const QModelIndex& proxy_qmi) /*override*/ {
         if (_proxy_qmi_is_scope_parent(proxy_qmi))
            return false;
         return QIdentityProxyModel::removeColumns(i, count, proxy_qmi);
      }
      /*virtual*/ bool DKScopedProxyModel::removeRows(int i, int count, const QModelIndex& proxy_qmi) /*override*/ {
         if (_proxy_qmi_is_scope_parent(proxy_qmi))
            return false;
         return QIdentityProxyModel::removeRows(i, count, proxy_qmi);
      }
   #pragma endregion
#pragma endregion
#pragma region QAbstractProxyModel overrides
   /*virtual*/ QModelIndex DKScopedProxyModel::mapFromSource(const QModelIndex& source_qmi) const /*override*/ {
      if (!_has_scope())
         return {};
      if (this->_scope_is_or_contains(source_qmi))
         return _unchecked_map_from_source(source_qmi);
      return {};
   }
   /*virtual*/ QModelIndex DKScopedProxyModel::mapToSource(const QModelIndex& proxy_qmi) const /*override*/ {
      if (!_has_scope())
         return {};
      if (_proxy_qmi_is_scope_parent(proxy_qmi))
         return {};
      if (_proxy_qmi_is_scope(proxy_qmi))
         return this->_scope.index.value();
      //
      // We can't call `createIndex` on the source model. Qt cheats by having QIdentityProxyModel 
      // be a friend of `QAbstractItemModel`, so we'll cheat by subclassing QIdentityProxyModel.
      // 
      return QIdentityProxyModel::mapToSource(proxy_qmi);
   }
   /*virtual*/ void DKScopedProxyModel::setSourceModel(QAbstractItemModel* model) /*override*/ {
      auto* prior = this->sourceModel();
      if (model == prior)
         return;
      if (prior) {
         QObject::disconnect(prior, nullptr, this, nullptr);
      }

      // Invoke the QAbstractProxyModel super, but NOT the QIdentityProxyModel super.
      QAbstractProxyModel::setSourceModel(model);

      this->_scope.index = {};
      if (model) {
         #pragma region Column changes
            QObject::connect(model, &QAbstractItemModel::columnsAboutToBeInserted, this, [this](const QModelIndex& parent, int first, int last) {
               if (!_scope_is_or_contains(parent))
                  return;
               this->beginInsertColumns(_unchecked_map_from_source(parent), first, last);
            });
            QObject::connect(model, &QAbstractItemModel::columnsInserted, this, [this](const QModelIndex& parent, int first, int last) {
               if (!_scope_is_or_contains(parent))
                  return;
               this->endInsertColumns();
            });

            QObject::connect(model, &QAbstractItemModel::columnsAboutToBeMoved, this, [this](const QModelIndex& src_parent, int first, int last, const QModelIndex& dst_parent, int dst) {
               if (_scope_is_or_contains(src_parent)) {
                  if (_scope_is_or_contains(dst_parent)) {
                     this->beginMoveColumns(_unchecked_map_from_source(src_parent), first, last, _unchecked_map_from_source(dst_parent), dst);
                  } else {
                     this->beginRemoveColumns(_unchecked_map_from_source(src_parent), first, last);
                  }
               } else {
                  if (_scope_is_or_contains(dst_parent)) {
                     int count = last - first + 1;
                     this->beginInsertColumns(_unchecked_map_from_source(dst_parent), dst, dst + count - 1);
                  }
               }
            });
            QObject::connect(model, &QAbstractItemModel::columnsMoved, this, [this](const QModelIndex& src_parent, int first, int last, const QModelIndex& dst_parent, int dst) {
               if (_scope_is_or_contains(src_parent)) {
                  if (_scope_is_or_contains(dst_parent)) {
                     this->endMoveColumns();
                  } else {
                     this->endRemoveColumns();
                  }
               } else {
                  if (_scope_is_or_contains(dst_parent)) {
                     this->endInsertColumns();
                  }
               }
            });

            QObject::connect(model, &QAbstractItemModel::columnsAboutToBeRemoved, this, [this](const QModelIndex& parent, int first, int last) {
               if (_source_col_range_includes_scope(parent, first, last)) {
                  this->beginResetModel();
                  this->_scope.index.reset();
                  this->endResetModel();
                  return;
               }
               if (!_scope_is_or_contains(parent))
                  return;
               this->beginRemoveColumns(mapFromSource(parent), first, last);
            });
            QObject::connect(model, &QAbstractItemModel::columnsRemoved, this, [this](const QModelIndex& parent, int first, int last) {
               if (!_scope_is_or_contains(parent))
                  return;
               this->endRemoveColumns();
            });
         #pragma endregion
         #pragma region Row changes
            QObject::connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, [this](const QModelIndex& parent, int first, int last) {
               if (!_scope_is_or_contains(parent))
                  return;
               this->beginInsertRows(_unchecked_map_from_source(parent), first, last);
            });
            QObject::connect(model, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex& parent, int first, int last) {
               if (!_scope_is_or_contains(parent))
                  return;
               this->endInsertRows();
            });

            QObject::connect(model, &QAbstractItemModel::rowsAboutToBeMoved, this, [this](const QModelIndex& src_parent, int first, int last, const QModelIndex& dst_parent, int dst) {
               if (_scope_is_or_contains(src_parent)) {
                  if (_scope_is_or_contains(dst_parent)) {
                     this->beginMoveRows(_unchecked_map_from_source(src_parent), first, last, _unchecked_map_from_source(dst_parent), dst);
                  } else {
                     this->beginRemoveRows(_unchecked_map_from_source(src_parent), first, last);
                  }
               } else {
                  if (_scope_is_or_contains(dst_parent)) {
                     int count = last - first + 1;
                     this->beginInsertRows(_unchecked_map_from_source(dst_parent), dst, dst + count - 1);
                  }
               }
            });
            QObject::connect(model, &QAbstractItemModel::rowsMoved, this, [this](const QModelIndex& src_parent, int first, int last, const QModelIndex& dst_parent, int dst) {
               if (_scope_is_or_contains(src_parent)) {
                  if (_scope_is_or_contains(dst_parent)) {
                     this->endMoveRows();
                  } else {
                     this->endRemoveRows();
                  }
               } else {
                  if (_scope_is_or_contains(dst_parent)) {
                     this->endInsertRows();
                  }
               }
            });

            QObject::connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this](const QModelIndex& parent, int first, int last) {
               if (_source_row_range_includes_scope(parent, first, last)) {
                  this->beginResetModel();
                  this->_scope.index.reset();
                  this->endResetModel();
                  return;
               }
               if (!_scope_is_or_contains(parent))
                  return;
               this->beginRemoveRows(_unchecked_map_from_source(parent), first, last);
            });
            QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, [this](const QModelIndex& parent, int first, int last) {
               if (!_scope_is_or_contains(parent))
                  return;
               this->endRemoveRows();
            });
         #pragma endregion
         //
         QObject::connect(model, &QAbstractItemModel::layoutAboutToBeChanged, this, [this](const QList<QPersistentModelIndex>& parents, QAbstractItemModel::LayoutChangeHint hint) {
            {
               QList<QPersistentModelIndex> proxy_parents;
               for (const auto& qpmi : parents) {
                  if (!_scope_is_or_contains(qpmi))
                     continue;
                  proxy_parents.push_back(_unchecked_map_from_source(qpmi));
               }
               if (!proxy_parents.empty())
                  emit layoutAboutToBeChanged(proxy_parents, hint);
            }

            auto& list = this->_pending_layout_changes.emplace_back();
            for (const auto& qmi : this->persistentIndexList()) {
               auto& item = list.emplace_back();
               item.proxy  = qmi;
               item.source = this->mapToSource(qmi);
            }
         });
         QObject::connect(model, &QAbstractItemModel::layoutChanged, this, [this](const QList<QPersistentModelIndex>& parents, QAbstractItemModel::LayoutChangeHint hint) {
            auto& list = this->_pending_layout_changes.back();
            for (const auto& item : list) {
               this->changePersistentIndex(item.proxy, this->mapFromSource(item.source));
            }
            this->_pending_layout_changes.pop_back();

            {
               QList<QPersistentModelIndex> proxy_parents;
               for (const auto& qpmi : parents) {
                  if (!_scope_is_or_contains(qpmi))
                     continue;
                  proxy_parents.push_back(_unchecked_map_from_source(qpmi));
               }
               if (!proxy_parents.empty())
                  emit layoutChanged(proxy_parents, hint);
            }
         });
         //
         QObject::connect(model, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
            if (!_has_scope())
               return;
            if (!_scope_is_or_contains(topLeft)) {
               //
               // Verify that the root isn't among the siblings of topLeft.
               //
               if (_scope_is_source_root())
                  return;
               const auto& scope = this->_scope.index.value();
               if (this->sourceModel()->sibling(topLeft.row(), topLeft.column(), scope) != topLeft)
                  return;
            }
            emit dataChanged(_unchecked_map_from_source(topLeft), _unchecked_map_from_source(bottomRight), roles);
         });
         QObject::connect(model, &QAbstractItemModel::headerDataChanged, this, &QAbstractItemModel::headerDataChanged);
      }
   }
#pragma endregion

bool DKScopedProxyModel::_scope_is_source_root() const noexcept {
   assert(_has_scope());
   return !this->_scope.index.value().isValid(); // invalid index is assumed to be model root
}

bool DKScopedProxyModel::_source_col_range_includes_scope(const QModelIndex& source_parent_qmi, int first, int last) const {
   if (!_has_scope())
      return false;
   const auto& scope = this->_scope.index.value();
   if (scope.parent() != source_parent_qmi)
      return false;
   auto i = scope.column();
   return first <= i && i <= last;
}
bool DKScopedProxyModel::_source_row_range_includes_scope(const QModelIndex& source_parent_qmi, int first, int last) const {
   if (!_has_scope())
      return false;
   const auto& scope = this->_scope.index.value();
   if (scope.parent() != source_parent_qmi)
      return false;
   auto i = scope.row();
   return first <= i && i <= last;
}

bool DKScopedProxyModel::_scope_is_or_contains(const QModelIndex& source_qmi) const {
   if (!_has_scope())
      return false;
   if (_scope_is_source_root())
      return true;
   const auto& scope = this->_scope.index.value();
   if (scope == source_qmi)
      return true;
   return _scope_contains(source_qmi);
}
bool DKScopedProxyModel::_scope_contains(const QModelIndex& source_qmi) const {
   if (!_has_scope())
      return false;
   if (_scope_is_source_root())
      return true;
   const auto& scope = this->_scope.index.value();
   auto parent = source_qmi.parent();
   for (; parent.isValid(); parent = parent.parent()) {
      if (parent == scope)
         return true;
   }
   return false;
}

bool DKScopedProxyModel::_proxy_scope_qmi_has_parent() const {
   return this->_scope.visible && _has_scope() && !_scope_is_source_root();
}
QModelIndex DKScopedProxyModel::_proxy_qmi_of_scope(int col) const {
   if (_proxy_scope_qmi_has_parent()) {
      return this->createIndex(0, col, (void*)this);
   } else {
      return {};
   }
}
bool DKScopedProxyModel::_proxy_qmi_is_scope(const QModelIndex& proxy_qmi) const {
   if (_proxy_scope_qmi_has_parent()) {
      //
      // Check for output of `_child_qmi_of_visible_root`.
      //
      if (proxy_qmi.row() != 0)
         return false;
      if (proxy_qmi.column() != 0)
         return false;
      return proxy_qmi.internalPointer() == this;
   } else {
      return !proxy_qmi.isValid();
   }
}
bool DKScopedProxyModel::_proxy_qmi_is_scope_parent(const QModelIndex& proxy_qmi) const {
   return _proxy_scope_qmi_has_parent() && !proxy_qmi.isValid();
}

// Use when you already know `source_qmi` is inside of the root model.
QModelIndex DKScopedProxyModel::_unchecked_map_from_source(const QModelIndex& source_qmi) const {
   const auto& scope = this->_scope.index.value();
   if (_proxy_scope_qmi_has_parent()) {
      if (source_qmi == scope.parent()) {
         return {};
      }
      if (source_qmi == scope && scope.isValid()) {
         return _proxy_qmi_of_scope(source_qmi.column());
      }
   } else {
      if (source_qmi == scope) {
         return {};
      }
   }
   return this->createIndex(source_qmi.row(), source_qmi.column(), source_qmi.internalPointer());
}