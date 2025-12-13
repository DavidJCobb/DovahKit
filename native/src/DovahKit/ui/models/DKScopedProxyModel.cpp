#include "./DKScopedProxyModel.h"

std::optional<QModelIndex> DKScopedProxyModel::rootIndex() const {
   return this->_root_index;
}
void DKScopedProxyModel::setRootIndex(const std::optional<QModelIndex>& opt_source_qmi) {
   if (!opt_source_qmi.has_value()) {
      if (!this->_root_index.has_value())
         return;
      this->beginResetModel();
      this->_root_index = opt_source_qmi;
      this->endResetModel();
      return;
   }
   const auto& source_qmi = opt_source_qmi.value();
   assert(source_qmi.model() == this->sourceModel());
   if (this->_root_index == source_qmi)
      return;
   this->beginResetModel();
   this->_root_index = source_qmi;
   this->endResetModel();
}

bool DKScopedProxyModel::isRootVisible() const {
   return this->_root_is_visible;
}
void DKScopedProxyModel::setRootVisible(bool v) {
   if (v == this->_root_is_visible)
      return;
   if (!this->sourceModel() || !this->_root_index.has_value()) {
      this->_root_is_visible = v;
      return;
   }
   this->beginResetModel();
   this->_root_is_visible = v;
   this->endResetModel();
}

#pragma region QAbstractItemModel overrides
   /*virtual*/ Qt::ItemFlags DKScopedProxyModel::flags(const QModelIndex& qmi) const /*override*/ {
      if (_proxy_qmi_is_super_root(qmi)) {
         return Qt::ItemFlag::ItemIsEnabled;
      }
      return QIdentityProxyModel::flags(qmi);
   }
   /*virtual*/ QModelIndex DKScopedProxyModel::index(int row, int column, const QModelIndex& proxy_parent_qmi) const /*override*/ {
      if (!this->_root_index.has_value())
         return {};
      const auto& source_root_qmi = this->_root_index.value();
      if (_proxy_qmi_is_super_root(proxy_parent_qmi)) {
         if (row != 0 || column != 0)
            return {};
         return this->mapFromSource(source_root_qmi);
      }
      return QIdentityProxyModel::index(row, column, proxy_parent_qmi);
   }
   /*virtual*/ int DKScopedProxyModel::columnCount(const QModelIndex& proxy_qmi) const /*override*/ {
      const auto* source_model = this->sourceModel();
      if (!source_model)
         return 0;
      if (!this->_root_index.has_value())
         return 0;
      if (_proxy_qmi_is_super_root(proxy_qmi))
         return 1;
      if (_proxy_qmi_is_root(proxy_qmi))
         return QIdentityProxyModel::columnCount(this->_root_index.value());
      return QIdentityProxyModel::columnCount(proxy_qmi);
   }
   /*virtual*/ int DKScopedProxyModel::rowCount(const QModelIndex& proxy_qmi) const /*override*/ {
      const auto* source_model = this->sourceModel();
      if (!source_model)
         return 0;
      if (!this->_root_index.has_value())
         return 0;
      if (_proxy_qmi_is_super_root(proxy_qmi))
         return 1;
      if (_proxy_qmi_is_root(proxy_qmi))
         return QIdentityProxyModel::rowCount(this->_root_index.value());
      return QIdentityProxyModel::rowCount(proxy_qmi);
   }
   /*virtual*/ QModelIndex DKScopedProxyModel::parent(const QModelIndex& proxy_qmi) const /*override*/ {
      if (!proxy_qmi.isValid())
         return {};
      if (_proxy_qmi_is_super_root(proxy_qmi))
         return {};
      return QIdentityProxyModel::parent(proxy_qmi);
   }
   /*virtual*/ QModelIndex DKScopedProxyModel::sibling(int row, int col, const QModelIndex& proxy_qmi) const /*override*/ {
      if (!proxy_qmi.isValid())
         return {};
      if (this->_root_is_visible) {
         if (_proxy_qmi_is_root(proxy_qmi)) {
            if (row == 0 && col == 0)
               return proxy_qmi;
            return {};
         }
      }
      return QIdentityProxyModel::sibling(row, col, proxy_qmi);
   }
   #pragma region Modify hierarchy
      /*virtual*/ bool DKScopedProxyModel::dropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) /*override*/ {
         if (_proxy_qmi_is_super_root(parent))
            return false;
         return QIdentityProxyModel::dropMimeData(mime, action, row, column, parent);
      }
      /*virtual*/ bool DKScopedProxyModel::insertColumns(int i, int count, const QModelIndex& proxy_qmi) /*override*/ {
         if (_proxy_qmi_is_super_root(proxy_qmi))
            return false;
         return QIdentityProxyModel::insertColumns(i, count, proxy_qmi);
      }
      /*virtual*/ bool DKScopedProxyModel::insertRows(int i, int count, const QModelIndex& proxy_qmi) /*override*/ {
         if (_proxy_qmi_is_super_root(proxy_qmi))
            return false;
         return QIdentityProxyModel::insertRows(i, count, proxy_qmi);
      }
      /*virtual*/ bool DKScopedProxyModel::moveColumns(const QModelIndex& from_parent, int first, int last, const QModelIndex& to_parent, int to) /*override*/ {
         if (_proxy_qmi_is_super_root(from_parent))
            return false;
         if (_proxy_qmi_is_super_root(to_parent))
            return false;
         return QIdentityProxyModel::moveColumns(from_parent, first, last, to_parent, to);
      }
      /*virtual*/ bool DKScopedProxyModel::moveRows(const QModelIndex& from_parent, int first, int last, const QModelIndex& to_parent, int to) /*override*/ {
         if (_proxy_qmi_is_super_root(from_parent))
            return false;
         if (_proxy_qmi_is_super_root(to_parent))
            return false;
         return QIdentityProxyModel::moveColumns(from_parent, first, last, to_parent, to);
      }
      /*virtual*/ bool DKScopedProxyModel::removeColumns(int i, int count, const QModelIndex& proxy_qmi) /*override*/ {
         if (_proxy_qmi_is_super_root(proxy_qmi))
            return false;
         return QIdentityProxyModel::removeColumns(i, count, proxy_qmi);
      }
      /*virtual*/ bool DKScopedProxyModel::removeRows(int i, int count, const QModelIndex& proxy_qmi) /*override*/ {
         if (_proxy_qmi_is_super_root(proxy_qmi))
            return false;
         return QIdentityProxyModel::removeRows(i, count, proxy_qmi);
      }
   #pragma endregion
#pragma endregion
#pragma region QAbstractProxyModel overrides
   /*virtual*/ QModelIndex DKScopedProxyModel::mapFromSource(const QModelIndex& source_qmi) const /*override*/ {
      if (!this->_root_index.has_value())
         return {};
      const auto& root_qmi = this->_root_index.value();
      if (this->_root_is_visible) {
         if (source_qmi == root_qmi.parent())
            return {};
      }
      if (this->_root_is_or_contains(source_qmi))
         return _unchecked_map_from_source(source_qmi);
      return {};
   }
   /*virtual*/ QModelIndex DKScopedProxyModel::mapToSource(const QModelIndex& proxy_qmi) const /*override*/ {
      if (!this->_root_index.has_value())
         return {};
      if (_proxy_qmi_is_super_root(proxy_qmi))
         return {};
      const auto& root_qmi = this->_root_index.value();
      if (this->_root_is_visible) {
         if (!proxy_qmi.isValid())
            return {};
         if (_proxy_qmi_is_root(proxy_qmi)) {
            return root_qmi;
         }
      } else {
         if (!proxy_qmi.isValid())
            return root_qmi;
      }
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

      this->_root_index = {};
      if (model) {
         #pragma region Column changes
            QObject::connect(model, &QAbstractItemModel::columnsAboutToBeInserted, this, [this](const QModelIndex& parent, int first, int last) {
               if (!_root_is_or_contains(parent))
                  return;
               this->beginInsertColumns(_unchecked_map_from_source(parent), first, last);
            });
            QObject::connect(model, &QAbstractItemModel::columnsInserted, this, [this](const QModelIndex& parent, int first, int last) {
               if (!_root_is_or_contains(parent))
                  return;
               this->endInsertColumns();
            });

            QObject::connect(model, &QAbstractItemModel::columnsAboutToBeMoved, this, [this](const QModelIndex& src_parent, int first, int last, const QModelIndex& dst_parent, int dst) {
               if (_root_is_or_contains(src_parent)) {
                  if (_root_is_or_contains(dst_parent)) {
                     this->beginMoveColumns(_unchecked_map_from_source(src_parent), first, last, _unchecked_map_from_source(dst_parent), dst);
                  } else {
                     this->beginRemoveColumns(_unchecked_map_from_source(src_parent), first, last);
                  }
               } else {
                  if (_root_is_or_contains(dst_parent)) {
                     int count = last - first + 1;
                     this->beginInsertColumns(_unchecked_map_from_source(dst_parent), dst, dst + count - 1);
                  }
               }
            });
            QObject::connect(model, &QAbstractItemModel::columnsMoved, this, [this](const QModelIndex& src_parent, int first, int last, const QModelIndex& dst_parent, int dst) {
               if (_root_is_or_contains(src_parent)) {
                  if (_root_is_or_contains(dst_parent)) {
                     this->endMoveColumns();
                  } else {
                     this->endRemoveColumns();
                  }
               } else {
                  if (_root_is_or_contains(dst_parent)) {
                     this->endInsertColumns();
                  }
               }
            });

            QObject::connect(model, &QAbstractItemModel::columnsAboutToBeRemoved, this, [this](const QModelIndex& parent, int first, int last) {
               if (_col_range_includes_root(parent, first, last)) {
                  this->beginResetModel();
                  this->_root_index.reset();
                  this->endResetModel();
                  return;
               }
               if (!_root_is_or_contains(parent))
                  return;
               this->beginRemoveColumns(mapFromSource(parent), first, last);
            });
            QObject::connect(model, &QAbstractItemModel::columnsRemoved, this, [this](const QModelIndex& parent, int first, int last) {
               if (!_root_is_or_contains(parent))
                  return;
               this->endRemoveColumns();
            });
         #pragma endregion
         #pragma region Row changes
            QObject::connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, [this](const QModelIndex& parent, int first, int last) {
               if (!_root_is_or_contains(parent))
                  return;
               this->beginInsertRows(_unchecked_map_from_source(parent), first, last);
            });
            QObject::connect(model, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex& parent, int first, int last) {
               if (!_root_is_or_contains(parent))
                  return;
               this->endInsertRows();
            });

            QObject::connect(model, &QAbstractItemModel::rowsAboutToBeMoved, this, [this](const QModelIndex& src_parent, int first, int last, const QModelIndex& dst_parent, int dst) {
               if (_root_is_or_contains(src_parent)) {
                  if (_root_is_or_contains(dst_parent)) {
                     this->beginMoveRows(_unchecked_map_from_source(src_parent), first, last, _unchecked_map_from_source(dst_parent), dst);
                  } else {
                     this->beginRemoveRows(_unchecked_map_from_source(src_parent), first, last);
                  }
               } else {
                  if (_root_is_or_contains(dst_parent)) {
                     int count = last - first + 1;
                     this->beginInsertRows(_unchecked_map_from_source(dst_parent), dst, dst + count - 1);
                  }
               }
            });
            QObject::connect(model, &QAbstractItemModel::rowsMoved, this, [this](const QModelIndex& src_parent, int first, int last, const QModelIndex& dst_parent, int dst) {
               if (_root_is_or_contains(src_parent)) {
                  if (_root_is_or_contains(dst_parent)) {
                     this->endMoveRows();
                  } else {
                     this->endRemoveRows();
                  }
               } else {
                  if (_root_is_or_contains(dst_parent)) {
                     this->endInsertRows();
                  }
               }
            });

            QObject::connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this](const QModelIndex& parent, int first, int last) {
               if (_row_range_includes_root(parent, first, last)) {
                  this->beginResetModel();
                  this->_root_index.reset();
                  this->endResetModel();
                  return;
               }
               if (!_root_is_or_contains(parent))
                  return;
               this->beginRemoveRows(_unchecked_map_from_source(parent), first, last);
            });
            QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, [this](const QModelIndex& parent, int first, int last) {
               if (!_root_is_or_contains(parent))
                  return;
               this->endRemoveRows();
            });
         #pragma endregion
         //
         QObject::connect(model, &QAbstractItemModel::layoutAboutToBeChanged, this, [this](const QList<QPersistentModelIndex>& parents, QAbstractItemModel::LayoutChangeHint hint) {
            {
               QList<QPersistentModelIndex> proxy_parents;
               for (const auto& qpmi : parents) {
                  if (!_root_is_or_contains(qpmi))
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
                  if (!_root_is_or_contains(qpmi))
                     continue;
                  proxy_parents.push_back(_unchecked_map_from_source(qpmi));
               }
               if (!proxy_parents.empty())
                  emit layoutChanged(proxy_parents, hint);
            }
         });
         //
         QObject::connect(model, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
            if (!this->_root_is_or_contains(topLeft)) {
               //
               // Verify that the root isn't among the siblings of topLeft.
               //
               if (!this->_root_index.has_value())
                  return;
               const auto& root_qmi = this->_root_index.value();
               if (!root_qmi.isValid())
                  return;
               if (this->sourceModel()->sibling(topLeft.row(), topLeft.column(), root_qmi) != topLeft)
                  return;
            }
            emit dataChanged(_unchecked_map_from_source(topLeft), _unchecked_map_from_source(bottomRight), roles);
         });
         QObject::connect(model, &QAbstractItemModel::headerDataChanged, this, &QAbstractItemModel::headerDataChanged);
      }
   }
#pragma endregion

bool DKScopedProxyModel::_col_range_includes_root(const QModelIndex& source_parent_qmi, int first, int last) const {
   if (!this->_root_index.has_value())
      return false;
   const auto& root_qmi = this->_root_index.value();
   if (root_qmi.parent() != source_parent_qmi)
      return false;
   auto i = root_qmi.column();
   return first <= i && i <= last;
}
bool DKScopedProxyModel::_row_range_includes_root(const QModelIndex& source_parent_qmi, int first, int last) const {
   if (!this->_root_index.has_value())
      return false;
   const auto& root_qmi = this->_root_index.value();
   if (root_qmi.parent() != source_parent_qmi)
      return false;
   auto i = root_qmi.row();
   return first <= i && i <= last;
}

bool DKScopedProxyModel::_root_is_or_contains(const QModelIndex& source_qmi) const {
   if (!this->_root_index.has_value())
      return false;
   const auto& root_qmi = this->_root_index.value();
   if (!root_qmi.isValid()) // invalid index is assumed to be model root
      return true;
   if (root_qmi == source_qmi)
      return true;
   return this->_root_contains(source_qmi);
}
bool DKScopedProxyModel::_root_contains(const QModelIndex& source_qmi) const {
   if (!this->_root_index.has_value())
      return false;
   const auto& root_qmi = this->_root_index.value();
   if (!root_qmi.isValid()) // invalid index is assumed to be model root
      return true;

   auto parent = source_qmi.parent();
   for (; parent.isValid(); parent = parent.parent()) {
      if (parent == root_qmi)
         return true;
   }
   return false;
}

bool DKScopedProxyModel::_proxy_qmi_is_root(const QModelIndex& proxy_qmi) const {
   if (this->_root_is_visible) {
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
bool DKScopedProxyModel::_proxy_qmi_is_super_root(const QModelIndex& qmi) const {
   return
      this->_root_is_visible
   && this->_root_index.has_value()
   && this->_root_index.value().isValid() // if the root index is the source model's root, then there is no super-root.
   && !qmi.isValid()
   ;
}

// Use when you already know `source_qmi` is inside of the root model.
QModelIndex DKScopedProxyModel::_unchecked_map_from_source(const QModelIndex& source_qmi) const {
   if (!this->_root_index.has_value())
      return {};
   const auto& root_qmi = this->_root_index.value();
   if (this->_root_is_visible) {
      if (source_qmi == root_qmi.parent()) {
         return {};
      }
      if (source_qmi == root_qmi && root_qmi.isValid()) {
         return _child_qmi_of_visible_root(source_qmi.column());
      }
   } else {
      if (source_qmi == root_qmi) {
         return {};
      }
   }
   return this->createIndex(source_qmi.row(), source_qmi.column(), source_qmi.internalPointer());
}

QModelIndex DKScopedProxyModel::_child_qmi_of_visible_root(int col) const {
   //
   // Use `this` as the internal pointer to avoid any confusion with source-model QMIs' internal pointers.
   //
   return this->createIndex(0, col, (void*)this);
}
bool DKScopedProxyModel::_is_parent_of_visible_root(const QModelIndex& proxy_qmi) const {
   if (!this->_root_is_visible)
      return false;
   return proxy_qmi.internalPointer() == this && proxy_qmi.model() == this;
}