#include "./RacePhonemeMorphsModel.h"

RacePhonemeMorphsModel::RacePhonemeMorphsModel(QObject* parent) : QAbstractItemModel(parent) {
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex RacePhonemeMorphsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_morphs.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex RacePhonemeMorphsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex RacePhonemeMorphsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         return this->index(row, column, {});
      }
      /*virtual*/ int RacePhonemeMorphsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_morphs.size();
      }
      /*virtual*/ int RacePhonemeMorphsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant RacePhonemeMorphsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_morphs.size())
            return {};
         auto& src = this->_morphs[index.row()];
         switch (index.column()) {
            case Column::Name:
               if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
                  return src.name;
               }
               break;
            case Column::Weight:
               if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
                  return QString::number(src.weight, 'f');
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags RacePhonemeMorphsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
      /*virtual*/ QVariant RacePhonemeMorphsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case Column::Name:
               return tr("Target Name");
            case Column::Weight:
               return tr("Weight");
         }
         return {};
      }
#pragma endregion

void RacePhonemeMorphsModel::setAllMorphNames(const std::vector<std::string>& src) {
   size_t size       = src.size();
   size_t size_prior = this->_morphs.size();
   bool   shortening = size < size_prior;
   if (size != size_prior) {
      if (shortening) {
         this->beginRemoveRows({}, size, this->_morphs.size() - 1);
      } else {
         this->beginInsertRows({}, this->_morphs.size(), size - 1);
      }
   }

   this->_morphs.resize(size);
   for (size_t i = 0; i < size; ++i) {
      this->_morphs[i].name = QString::fromStdString(src[i]);
   }

   if (size != size_prior) {
      if (shortening) {
         this->endRemoveRows();
      } else {
         this->endInsertRows();
      }
   }

   auto qmi = QModelIndex{};
   auto tl  = this->index(0,        Column::Name, qmi);
   auto br  = this->index(size - 1, Column::Name, qmi);
   emit dataChanged(tl, br);
}
//
QString RacePhonemeMorphsModel::morphName(size_t i) const {
   if (i >= this->_morphs.size())
      return "";
   return this->_morphs[i].name;
}
void RacePhonemeMorphsModel::setMorphName(size_t i, QString v) {
   if (i >= this->_morphs.size())
      return;
   auto& dst = this->_morphs[i];
   dst.name = v;
   
   auto qmi = this->index(i, Column::Name, {});
   emit dataChanged(qmi, qmi);
}

void RacePhonemeMorphsModel::setAllMorphWeights(const std::vector<float>& src) {
   size_t size = this->_morphs.size();
   size_t end  = std::min(src.size(), size);

   size_t i = 0;
   for (; i < end; ++i) {
      this->_morphs[i].weight = src[i];
   }
   for (; i < size; ++i) {
      this->_morphs[i].weight = 0;
   }
   
   auto qmi = QModelIndex{};
   auto tl  = this->index(0,        Column::Weight, qmi);
   auto br  = this->index(size - 1, Column::Weight, qmi);
   emit dataChanged(tl, br);
}
//
float RacePhonemeMorphsModel::morphWeight(size_t i) const {
   if (i >= this->_morphs.size())
      return 0;
   return this->_morphs[i].weight;
}
void RacePhonemeMorphsModel::setMorphWeight(size_t i, float v) {
   if (i >= this->_morphs.size())
      return;
   auto& dst = this->_morphs[i];
   dst.weight = v;
   
   auto qmi = this->index(i, Column::Weight, {});
   emit dataChanged(qmi, qmi);
}

QModelIndex RacePhonemeMorphsModel::addMorph() {
   auto&  list = this->_morphs;
   size_t size = list.size();
   this->beginInsertRows({}, size, size);
   auto& item = list.emplace_back();
   item.name = _get_new_morph_name();
   this->endInsertRows();
   return this->index(size, 0, {});
}

void RacePhonemeMorphsModel::deleteMorph(size_t i) {
   if (i >= this->_morphs.size())
      return;
   this->beginRemoveRows({}, i, i);
   this->_morphs.erase(this->_morphs.begin() + i);
   this->endRemoveRows();
}
void RacePhonemeMorphsModel::deleteMorph(const QModelIndex& qmi) {
   if (!qmi.isValid() || qmi.model() != this)
      return;
   this->deleteMorph(qmi.row());
}

[[nodiscard]] std::vector<float> RacePhonemeMorphsModel::allMorphWeights() const {
   std::vector<float> out;

   auto&  src  = this->_morphs;
   size_t size = src.size();
   out.resize(size);
   for (size_t i = 0; i < size; ++i)
      out[i] = src[i].weight;
   return out;
}
[[nodiscard]] std::vector<QString> RacePhonemeMorphsModel::allMorphNames() const {
   std::vector<QString> out;

   auto&  src  = this->_morphs;
   size_t size = src.size();
   out.resize(size);
   for (size_t i = 0; i < size; ++i)
      out[i] = src[i].name;
   return out;
}

QString RacePhonemeMorphsModel::_get_new_morph_name() const {
   {
      bool taken = false;
      for (auto& item : this->_morphs) {
         if (item.name == "new") {
            taken = true;
            break;
         }
      }
      if (!taken)
         return "new";
   }

   constexpr const size_t failsafe_max_count = 100;
   //
   for (size_t i = 0; i < failsafe_max_count; ++i) {
      auto name  = QString("new%1").arg(i);
      bool taken = false;
      for (auto& item : this->_morphs) {
         if (item.name == name) {
            taken = true;
            break;
         }
      }
      if (!taken)
         return name;
   }

   return "new?";
}