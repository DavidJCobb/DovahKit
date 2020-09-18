#include "use_info_list.h"
#include <QHeaderView>
#include <QLineEdit>
#include "../../../helpers/qt/strings.h"
#include "../../../editor/core.h"
#include "../../../editor/open_window_for_form.h"

#pragma region FormUseInfoListModel
FormUseInfoListModelItem::FormUseInfoListModelItem(const dovah::use_info_entry* source) {
   bool is_reference = (source->flags & data_t::flag::i_am_base_form_of) != 0;
   //
   this->flags       = source->flags;
   this->countUsed   = source->refcount;
   this->countPlaced = 0;
   if (is_reference) {
      --this->countUsed;
      ++this->countPlaced;
   }
   if (auto stub = source->other) {
      this->otherStub = stub;
      this->otherID   = stub->formID;
      this->otherType = stub->formType;
      this->editorID  = stub->get_editor_id();
      //
      uint32_t signature = dovah::form_type_info::lookup(this->otherType).signature;
      this->signature = cobb::qt::four_cc_to_string(signature);
      //
      if (is_reference) {
         auto parent = stub->get_parent_form();
         if (parent) {
            auto name = parent->get_editor_id();
            auto id   = QString("%1").arg(parent->formID, 8, 16, QChar('0')).toUpper();
            this->parentCell = QString("[CELL:%1]").arg(id);
            if (name && name[0]) {
               this->parentCell += name;
            } else {
               auto world = parent->get_parent_form();
               if (world) {
                  auto id = QString("%1").arg(world->formID, 8, 16, QChar('0')).toUpper();
                  QString s = QString("[WRLD:%1]%2").arg(id).arg(world->get_editor_id());
                  this->parentCell = QString("%2 in %1").arg(s).arg(this->parentCell);
               }
            }
         }
      }
      //
   }
}

inline int FormUseInfoListModelRoot::indexOf(item_type* item) const noexcept {
   int size = this->_children.size();
   for (int i = 0; i < size; ++i)
      if (this->_children[i] == item)
         return i;
   return -1;
}

QModelIndex FormUseInfoListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->root->child(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex FormUseInfoListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int FormUseInfoListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->root->childCount();
}
int FormUseInfoListModel::columnCount(const QModelIndex& item) const {
   if (this->mode == relationship_mode::base_form_only) {
      return 3;
   }
   return 4;
}
Qt::ItemFlags FormUseInfoListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}
QVariant FormUseInfoListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item   = (item_type*)index.internalPointer();
   auto column = index.column();
   switch (role) {
      case Qt::DisplayRole:
         switch (column) {
            case 0:
               return item->signature;
            case 1:
               return QString("%1").arg(item->otherID, 8, 16, QChar('0')).toUpper();
            case 2:
               if (this->mode == relationship_mode::base_form_only) {
                  return item->parentCell;
               }
               return item->editorID;
            case 3:
               switch (this->mode) {
                  case relationship_mode::general_only:
                     return item->countUsed;
                  case relationship_mode::base_form_only:
                     return item->countPlaced;
               }
               return 0;
         }
         break;
   }
   return QVariant();
}
//
QVariant FormUseInfoListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case 0: return tr("Type",      "use info report");
            case 1: return tr("Form ID",   "use info report");
            case 2:
               if (this->mode == relationship_mode::base_form_only) {
                  return tr("Parent Cell", "use info report");
               }
               return tr("Editor ID", "use info report");
            case 3:
               if (this->mode == relationship_mode::base_form_only) {
                  return tr("No. Placed", "use info report");
               }
               return tr("Use Count", "use info report");
         }
         break;
   }
   return QVariant();
}

void FormUseInfoListModel::clear() {
   this->beginResetModel();
   this->root->clear();
   this->endResetModel();
}
void FormUseInfoListModel::build(const dovah::form_stub* used) {
   this->clear();
   if (!used)
      return;
   //
   QVector<item_type*> insertions;
   //
   using _ue_flag = dovah::use_info_entry::flag;
   for (auto& pair : used->inbound) {
      auto& entry = pair.second;
      switch (this->mode) {
         case relationship_mode::general_only:
            if (entry.flags & (_ue_flag::i_am_base_form_of | _ue_flag::i_am_reference_of))
               if (entry.refcount <= 1)
                  continue;
         case relationship_mode::base_form_only:
            if (!(entry.flags & (_ue_flag::i_am_base_form_of | _ue_flag::i_am_reference_of)))
               continue;
      }
      auto item = new item_type(&entry);
      insertions.push_back(item);
   }
   auto first_inserted = this->root->childCount();
   auto last_inserted  = first_inserted + insertions.size();
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted); // we're not passing the count, we're passing the index of the last row. how annoying.
   for(auto* item : insertions)
      this->root->_children.push_back(item);
   this->endInsertRows();
}
void FormUseInfoListModel::setRelationshipMode(relationship_mode mode) noexcept {
   this->mode = mode;
}
#pragma endregion

#pragma region FormUseInfoList
FormUseInfoList::FormUseInfoList(QWidget* parent) : QTableView(parent) {
   this->setModel(new model_type);
   this->verticalHeader()->setDefaultSectionSize(0);
   this->sortByColumn(0, Qt::AscendingOrder);
   //
   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->resizeSection(0, metrics.boundingRect("XXXX").width() * 1.5F + 4);
   header->resizeSection(1, 4);
   header->resizeSection(3, metrics.boundingRect("Use Count").width() * 1.5F + 4);
   header->setSectionResizeMode(0, QHeaderView::Interactive);
   header->setSectionResizeMode(1, QHeaderView::Interactive);
   header->setSectionResizeMode(2, QHeaderView::Stretch);
   header->setSectionResizeMode(3, QHeaderView::Interactive);
   //
   QObject::connect(this, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      if (!index.isValid())
         return;
      auto data = (model_item_type*)index.internalPointer();
      if (data && data->otherStub)
         open_window_for_form(data->otherStub, this);
   });
   //
   auto  model  = (model_type*)this->model();
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() { this->setTarget(nullptr); });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      //
      // TODO: If (stub) was one of the forms using our target, then update our model.
      //
   });
};
FormUseInfoList::relationship_mode FormUseInfoList::relationshipMode() const noexcept {
   auto model = (model_type*)this->model();
   if (!model)
      return relationship_mode::invalid;
   return model->relationshipMode();
}
void FormUseInfoList::setRelationshipMode(relationship_mode m) noexcept {
   auto model = (model_type*)this->model();
   if (!model)
      return;
   model->setRelationshipMode(m);
   model->build(this->target);
}
void FormUseInfoList::setTarget(const dovah::form_stub* stub) {
   this->target = stub;
   auto model = (model_type*)this->model();
   if (!model)
      return;
   model->build(stub);
}
void FormUseInfoList::build() {
   auto model = (model_type*)this->model();
   if (!model)
      return;
   model->build(this->target);
}
#pragma endregion