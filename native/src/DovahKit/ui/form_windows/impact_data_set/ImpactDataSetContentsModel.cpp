#include "./ImpactDataSetContentsModel.h"
#include "helpers/vectors/re_sort_item_within.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"

ImpactDataSetContentsModel::ImpactDataSetContentsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &ImpactDataSetContentsModel::_on_data_acquired);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ImpactDataSetContentsModel::_on_data_abandoned);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formCreated, this, [this](dovah::form_stub* stub) { this->_on_form_created(*stub); });
   QObject::connect(&editor, &DovahKitCore::formRenumbered, this, [this](dovah::form_stub* stub) { this->_on_form_renumbered(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &ImpactDataSetContentsModel::_on_all_forms_renumbered);
   if (editor.has_data()) {
      this->_on_data_acquired();
   }
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex ImpactDataSetContentsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex ImpactDataSetContentsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex ImpactDataSetContentsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int ImpactDataSetContentsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int ImpactDataSetContentsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant ImpactDataSetContentsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_data.size())
            return {};
         auto& src = this->_data[index.row()];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (index.column()) {
                  case Column::MaterialTypeName:
                     return QString::fromStdString(src.first->editorID);
                  case Column::MaterialTypeFormID:
                     return editor_helpers::form_id_to_string(src.first->formID);
                  case Column::ImpactDataName:
                     if (src.second) {
                        return QString::fromStdString(src.second->editorID);
                     }
                     break;
                  case Column::ImpactDataFormID:
                     if (src.second) {
                        return editor_helpers::form_id_to_string(src.second->formID);
                     }
                     break;
               }
               break;
            case FormStubRole:
               switch (index.column()) {
                  case Column::MaterialTypeName:
                  case Column::MaterialTypeFormID:
                     return QVariant::fromValue(src.first);
                  case Column::ImpactDataName:
                  case Column::ImpactDataFormID:
                     return QVariant::fromValue(src.second);
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags ImpactDataSetContentsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant ImpactDataSetContentsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      switch (section) {
         case Column::MaterialTypeName:
            return tr("Material Type");
         case Column::MaterialTypeFormID:
            return tr("Material Type Form ID");
         case Column::ImpactDataName:
            return tr("Impact Data");
         case Column::ImpactDataFormID:
            return tr("Impact Data Form ID");
      }
      return {};
   }
#pragma endregion

void ImpactDataSetContentsModel::importData(const backend_form_type& form) {
   this->beginResetModel();
   for (auto& item : this->_data) {
      item.second = nullptr;
   }
   for (auto& src : form.mappings) {
      auto* matt = src.material_type.get_form_stub();
      auto* data = src.impact_data.get_form_stub();
      if (matt->form_type != dovah::form_type::material_type)
         continue;
      if (!data || data->form_type != dovah::form_type::impact_data)
         continue;
      for (auto& dst : this->_data) {
         if (dst.first == matt) {
            dst.second = data;
            break;
         }
      }
   }
   this->endResetModel();
}
void ImpactDataSetContentsModel::exportData(backend_form_type& form) const {
   for (auto& item : form.mappings) {
      item.material_type.set(form, nullptr);
      item.impact_data.set(form, nullptr);
   }
   form.mappings.clear();

   for (auto& item : this->_data) {
      if (!item.second)
         continue;
      auto& dst = form.mappings.emplace_back();
      dst.material_type.set(form, item.first);
      dst.impact_data.set(form, item.second);
   }
}

QModelIndex ImpactDataSetContentsModel::indexOfMaterial(const dovah::form_stub& matt) {
   for (size_t i = 0; i < this->_data.size(); ++i)
      if (this->_data[i].first == &matt)
         return this->index(i, 0, {});
   return {};
}
void ImpactDataSetContentsModel::setImpactData(const dovah::form_stub& matt, dovah::form_stub* impact_data) {
   setImpactData(indexOfMaterial(matt), impact_data);
}
void ImpactDataSetContentsModel::setImpactData(QModelIndex material_qmi, dovah::form_stub* impact_data) {
   if (!material_qmi.isValid())
      return;
   this->_data[material_qmi.row()].second = impact_data;

   auto tl = material_qmi.siblingAtColumn(Column::ImpactDataName);
   auto br = material_qmi.siblingAtColumn(Column::ImpactDataFormID);
   emit dataChanged(tl, br);
}

void ImpactDataSetContentsModel::setAllUnsetTo(dovah::form_stub* impact_data) {
   if (!impact_data || impact_data->form_type != dovah::form_type::impact_data)
      return;
   for (size_t i = 0; i < this->_data.size(); ++i) {
      auto& item = this->_data[i];
      if (item.second)
         continue;
      item.second = impact_data;

      auto tl = this->index(i, Column::ImpactDataName, {});
      auto br = this->index(i, Column::ImpactDataFormID, {});
      emit dataChanged(tl, br);
   }
}

void ImpactDataSetContentsModel::_on_data_acquired() {
   this->beginResetModel();

   auto& editor = DovahKitCore::get();
   editor.for_each_form_of_type(dovah::form_type::material_type, [this](dovah::form_stub* stub) {
      assert(stub != nullptr);
      this->_data.push_back({ stub, nullptr });
      return false;
   });
   std::sort(
      this->_data.begin(),
      this->_data.end(),
      [](const auto& a, const auto& b) {
         return QString::localeAwareCompare(
            QString::fromStdString(a.first->editorID),
            QString::fromStdString(b.first->editorID)
         ) < 0;
      }
   );

   this->endResetModel();
}
void ImpactDataSetContentsModel::_on_data_abandoned() {
   this->beginResetModel();
   this->_data.clear();
   this->endResetModel();
}
void ImpactDataSetContentsModel::_on_form_created(dovah::form_stub& stub) {
   if (stub.form_type != dovah::form_type::material_type)
      return;

   QString editor_id = QString::fromStdString(stub.editorID);
   auto    it = _insertion_point_for(editor_id);
   auto    i  = std::distance(this->_data.begin(), it);
   this->beginInsertRows({}, i, i);
   this->_data.insert(it, { &stub, nullptr });
   this->endInsertRows();
}
void ImpactDataSetContentsModel::_on_form_deleted(dovah::form_stub& stub) {
   switch (stub.form_type) {
      case dovah::form_type::material_type:
         for (size_t i = 0; i < this->_data.size(); ++i) {
            auto& pair = this->_data[i];
            if (pair.first == &stub) {
               this->beginRemoveRows({}, i, i);
               this->_data.erase(this->_data.begin() + i);
               this->endRemoveRows();
               break;
            }
         }
         break;
      case dovah::form_type::impact_data:
         for (size_t i = 0; i < this->_data.size(); ++i) {
            auto& pair = this->_data[i];
            if (pair.second == &stub) {
               pair.second = nullptr;
               auto tl = this->index(i, Column::ImpactDataName, {});
               auto br = this->index(i, Column::ImpactDataFormID, {});
               emit dataChanged(tl, br);
            }
         }
         break;
   }
}
void ImpactDataSetContentsModel::_on_form_modified(dovah::form_stub& stub) {
   switch (stub.form_type) {
      case dovah::form_type::material_type:
         for (size_t i = 0; i < this->_data.size(); ++i) {
            auto& pair = this->_data[i];
            if (pair.first == &stub) {
               auto qmi = this->index(i, Column::MaterialTypeName, {});
               emit dataChanged(qmi, qmi);
               this->_re_sort_item(i);
               break;
            }
         }
         break;
      case dovah::form_type::impact_data:
         for (size_t i = 0; i < this->_data.size(); ++i) {
            auto& pair = this->_data[i];
            if (pair.second == &stub) {
               auto qmi = this->index(i, Column::ImpactDataName, {});
               emit dataChanged(qmi, qmi);
            }
         }
         break;
   }
}
void ImpactDataSetContentsModel::_on_form_renumbered(dovah::form_stub& stub) {
   switch (stub.form_type) {
      case dovah::form_type::material_type:
         for (size_t i = 0; i < this->_data.size(); ++i) {
            auto& pair = this->_data[i];
            if (pair.first == &stub) {
               auto qmi = this->index(i, Column::MaterialTypeFormID, {});
               emit dataChanged(qmi, qmi);
               break;
            }
         }
         break;
      case dovah::form_type::impact_data:
         for (size_t i = 0; i < this->_data.size(); ++i) {
            auto& pair = this->_data[i];
            if (pair.second == &stub) {
               auto qmi = this->index(i, Column::ImpactDataFormID, {});
               emit dataChanged(qmi, qmi);
            }
         }
         break;
   }
}
void ImpactDataSetContentsModel::_on_all_forms_renumbered() {
   auto size = this->_data.size();

   {
      auto tl = this->index(0, Column::MaterialTypeFormID, {});
      auto br = this->index(size - 1, Column::MaterialTypeFormID, {});
      emit dataChanged(tl, br);
   }
   {
      auto tl = this->index(0, Column::ImpactDataFormID, {});
      auto br = this->index(size - 1, Column::ImpactDataFormID, {});
      emit dataChanged(tl, br);
   }
}

decltype(ImpactDataSetContentsModel::_data)::iterator ImpactDataSetContentsModel::_insertion_point_for(QString material_type_editor_id) {
   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      material_type_editor_id,
      [](QString editor_id, const auto& item) -> bool {
         return QString::localeAwareCompare(editor_id, QString::fromStdString(item.first->editorID)) < 0;
      }
   );
}
void ImpactDataSetContentsModel::_re_sort_item(size_t from) {
   auto& list = this->_data;
   if (from >= list.size())
      return;
   bool moved     = false;
   auto editor_id = QString::fromStdString(list[from].first->editorID);
   cobb::vectors::re_sort_item_within(
      list,
      list.begin() + from,
      [editor_id](auto& a, auto& b) -> bool {
         return editor_id < QString::fromStdString(b.first->editorID);
      },
      [&moved, this, &list](decltype(_data)::iterator from_it, decltype(_data)::iterator to_it) {
         moved = true;
         size_t from  = std::distance(list.begin(), from_it);
         size_t to    = std::distance(list.begin(), to_it);
         this->beginMoveRows(
            {},
            from, // first to move
            from, // last  to move
            {},
            (to < from) ? to : to + 1 // Qt API design jank
         );
      }
   );
   if (moved)
      this->endMoveRows();
}