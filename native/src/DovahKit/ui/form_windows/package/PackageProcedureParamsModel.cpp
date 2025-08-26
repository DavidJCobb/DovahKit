#include "./PackageProcedureParamsModel.h"
#include "dovah/data/packages/procedure_type.h"

PackageProcedureParamsModel::PackageProcedureParamsModel(QObject* parent) : QAbstractItemModel(parent) {
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex PackageProcedureParamsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_params.size())
            return {};
         if (col < 0 || col >= this->columnCount())
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex PackageProcedureParamsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex PackageProcedureParamsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int PackageProcedureParamsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_params.size();
      }
      /*virtual*/ int PackageProcedureParamsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant PackageProcedureParamsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_params.size())
            return {};
         auto& src = this->_params[index.row()];

         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (index.column()) {
                  case Column::Name:
                     if (src.cached.name.isEmpty()) {
                        return tr("Package Data ID #%1").arg(src.unique_id);
                     }
                     return src.cached.name;
                  case Column::Value:
                     return src.cached.value;
                  case UniqueIDRole:
                     return src.unique_id;
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags PackageProcedureParamsModel::flags(const QModelIndex& index) const /*override*/ {
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant PackageProcedureParamsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      switch (section) {
         case Column::Name:
            return tr("Name");
         case Column::Value:
            return tr("Value");
      }
      return {};
   }
#pragma endregion

void PackageProcedureParamsModel::clear() {
   this->beginResetModel();
   this->_packdata_model = nullptr;
   this->_params.clear();
   this->endResetModel();
}

void PackageProcedureParamsModel::setPackdataModel(PackageDataModel* model) {
   if (this->_packdata_model == model)
      return;
   if (this->_packdata_model) {
      QObject::disconnect(this->_packdata_model.data(), nullptr, this, nullptr);
   }
   this->_packdata_model = model;
   if (model) {
      QObject::connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this, model](const QModelIndex& parent, int first, int last) {
         auto qmi            = model->index(first, 0, parent);
         auto unique_id_data = model->data(qmi, PackageDataModel::UniqueIDRole);
         if (!unique_id_data.isValid())
            return;
         uint8_t unique_id = unique_id_data.toInt();
         for (auto& item : this->_params) {
            if (item.unique_id == unique_id) {
               qWarning("PackageProcedureParamsModel: detected removal of a packdata while that data is still in use!");
               item.unique_id = 0xFF;
            }
         }
      });
      QObject::connect(model, &QAbstractItemModel::dataChanged, this, [this, model](const QModelIndex& qmi) {
         auto unique_id_data = model->data(qmi, PackageDataModel::UniqueIDRole);
         if (!unique_id_data.isValid())
            return;
         uint8_t unique_id = unique_id_data.toInt();

         QString name;
         QString value;
         bool grabbed = false;

         for (size_t i = 0; i < this->_params.size(); ++i) {
            auto& item = this->_params[i];
            if (item.unique_id == unique_id) {
               if (!grabbed) {
                  grabbed = true;
                  name  = model->data(qmi.siblingAtColumn(PackageDataModel::Column::Name), Qt::DisplayRole).toString();
                  value = model->data(qmi.siblingAtColumn(PackageDataModel::Column::Value), Qt::DisplayRole).toString();
               }
               item.cached.name  = name;
               item.cached.value = value;
            
               auto tl = this->index(i, 0, {});
               auto br = this->index(i, Column::__COUNT - 1, {});
               emit dataChanged(tl, br);
            }
         }
      });
   }
   this->_pull_package_data_names(true);
}

void PackageProcedureParamsModel::importData(const procedure_node_type& src) {
   this->importData(src.type, src.parameter_unique_ids);
}
void PackageProcedureParamsModel::exportData(procedure_node_type& dst) const {
   size_t size = this->_params.size();
   dst.parameter_unique_ids.resize(size);
   for (size_t i = 0; i < size; ++i) {
      dst.parameter_unique_ids[i] = this->_params[i].unique_id;
   }
}
//
void PackageProcedureParamsModel::importData(dovah::packages::procedure_type type, const std::vector<uint8_t>& src) {
   this->beginResetModel();
   this->_params.clear();
   
   if ((size_t)type < dovah::packages::all_procedure_type_info.size()) {
      const auto& info = dovah::packages::all_procedure_type_info[(size_t)type];
      this->_params.resize(info.param_count);

      size_t size = src.size();
      if (size > info.param_count)
         size = info.param_count;

      size_t i = 0;
      for (; i < size; ++i) {
         this->_params[i].unique_id = src[i];
      }
      for (; i < info.param_count; ++i) {
         this->_params[i].unique_id = 0xFF;
      }
   }
   this->_pull_package_data_names(false);
   this->endResetModel();
}
std::vector<uint8_t> PackageProcedureParamsModel::exportData() const {
   std::vector<uint8_t> dst;
   size_t size = this->_params.size();
   dst.resize(size);
   for (size_t i = 0; i < size; ++i) {
      dst[i] = this->_params[i].unique_id;
   }
   return dst;
}

void PackageProcedureParamsModel::_pull_package_data_names(bool emit_signals) {
   if (!this->_packdata_model) {
      for (size_t i = 0; i < this->_params.size(); ++i) {
         auto& item = this->_params[i];
         if (item.cached.name.isEmpty() && item.cached.value.isEmpty())
            continue;
         item.cached = {};

         auto tl = this->index(i, 0, {});
         auto br = this->index(i, Column::__COUNT - 1, {});
         emit dataChanged(tl, br);
      }
      return;
   }

   for (size_t i = 0; i < this->_params.size(); ++i) {
      auto& item      = this->_params[i];
      auto  unique_id = item.unique_id;
      auto  qmi       = this->_packdata_model->findUniqueID(unique_id);
      if (qmi.isValid()) {
         item.cached.name  = this->_packdata_model->data(qmi.siblingAtColumn(PackageDataModel::Column::Name), Qt::DisplayRole).toString();
         item.cached.value = this->_packdata_model->data(qmi.siblingAtColumn(PackageDataModel::Column::Value), Qt::DisplayRole).toString();
      } else {
         item.cached = {};
      }
   }
   auto tl = this->index(0, 0, {});
   auto br = this->index(this->_params.size() - 1, Column::__COUNT - 1, {});
   emit dataChanged(tl, br);
}