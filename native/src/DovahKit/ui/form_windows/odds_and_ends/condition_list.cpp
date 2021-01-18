#include "condition_list.h"
#include <QHeaderView>
#include "../../../editor/core.h"
#include "../../../dovah/form_stub.h"

void ConditionListModelItem::set_from(const source_t& cnd) {
   for (uint8_t i = 0; i < 2; ++i)
      if (this->get_argument_underlying_type(i) == dovah::loaded_forms::components::condition_info::arg_underlying_type::formID)
         this->parameters[i].form.unmanaged_set(nullptr);
   //
   this->type = cnd.type;
   this->compare_to_constant = cnd.compare_to_constant;
   this->compare_to_global.unmanaged_set(cnd.compare_to_global.get_form_stub());
   this->function      = cnd.function;
   this->run_on        = cnd.run_on;
   this->run_on_index  = cnd.run_on_index;
   this->run_on_reference.unmanaged_set(cnd.run_on_reference.get_form_stub());
   this->eventFunction = cnd.eventFunction;
   this->eventMember   = cnd.eventMember;
   this->eventFormID.unmanaged_set(cnd.eventFormID.get_form_stub());
   //
   for (uint8_t i = 0; i < 2; ++i) {
      using _u = dovah::loaded_forms::components::condition_info::arg_underlying_type;
      switch (cnd.get_argument_underlying_type(i)) {
         case _u::formID:
            this->parameters[i].form.unmanaged_set(cnd.parameters[i].form.get_form_stub());
            break;
         case _u::string:
            this->parameters[i].string = cnd.parameters[i].string;
            break;
         default:
            this->parameters[i].dword = cnd.parameters[i].dword;
            break;
      }
   }
}
void ConditionListModelItem::write_to(source_t& cnd, dovah::form_stub& owner) const {
   for (uint8_t i = 0; i < 2; ++i)
      if (cnd.get_argument_underlying_type(i) == dovah::loaded_forms::components::condition_info::arg_underlying_type::formID)
         cnd.parameters[i].form.set(owner, nullptr);
   //
   cnd.type = this->type;
   cnd.compare_to_constant = this->compare_to_constant;
   cnd.compare_to_global.set(owner, this->compare_to_global);
   cnd.function      = this->function;
   cnd.run_on        = this->run_on;
   cnd.run_on_index  = this->run_on_index;
   cnd.run_on_reference.set(owner, this->run_on_reference);
   cnd.eventFunction = this->eventFunction;
   cnd.eventMember   = this->eventMember;
   cnd.eventFormID.set(owner, this->eventFormID);
   //
   for (uint8_t i = 0; i < 2; ++i) {
      using _u = dovah::loaded_forms::components::condition_info::arg_underlying_type;
      switch (cnd.get_argument_underlying_type(i)) {
         case _u::formID:
            cnd.parameters[i].form.set(owner, this->parameters[i].form);
            break;
         case _u::string:
            cnd.parameters[i].string = this->parameters[i].string;
            break;
         default:
            cnd.parameters[i].dword = this->parameters[i].dword;
            break;
      }
   }
}
void ConditionListModelItem::sever_outbound_references_to(dovah::form_stub& target) noexcept {
   using namespace dovah::loaded_forms::components;
   //
   this->compare_to_global.unmanaged_clear_if(target);
   //
   auto func = condition_info::function::lookup_by_id(this->function);
   for (int i = 0; i < 2; i++) {
      auto& param = this->parameters[i];
      if (func && this->get_argument_underlying_type(i) == condition_info::arg_underlying_type::formID) {
         param.form.unmanaged_clear_if(target);
      }
   }
   //
   this->run_on_reference.unmanaged_clear_if(target);
   this->eventFormID.unmanaged_clear_if(target);
}
QString ConditionListModelItem::arguments_to_string() const noexcept {
   using _u = dovah::loaded_forms::components::condition_info::arg_underlying_type;
   //
   QString out;
   for (int i = 0; i < 2; ++i) {
      if (!out.isEmpty())
         out += QObject::tr(", ", "condition argument separator");
      auto& param = this->parameters[i];
      auto  under = this->get_argument_underlying_type(i);
      switch (under) {
         case _u::character:
            out += QChar(param.dword & 0xFF);
            continue;
         case _u::float32:
            out += QString("%1").arg(param.float32);
            continue;
         case _u::int_signed:
            static_assert(sizeof(qulonglong) >= sizeof(int32_t));
            out += QString("%1").arg(qlonglong(param.dword));
            continue;
         case _u::int_unsigned:
         case _u::quest_stage:
            static_assert(sizeof(qulonglong) >= sizeof(uint32_t));
            out += QString("%1").arg(qulonglong(param.dword));
            continue;
      }
      auto* type = this->get_argument_type(i);
      if (type) {
         out += type->name.c_str();
         out += ": ";
      }
   }
}

#pragma region ConditionListModel
ConditionListModel::ConditionListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &ConditionListModel::clear);
   QObject::connect(&editor, &DovahKitCore::formModified,         this, &ConditionListModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &ConditionListModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,       this, &ConditionListModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, [this]() { this->rebuild(this->worldspace); });
}

void ConditionListModel::formModified(const dovah::form_stub* stub) {
   if (stub->formType != dovah::form_type::cell)
      return;
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         item->update();
         auto root  = QModelIndex();
         auto start = this->index(i, 0, root);
         auto end   = this->index(i, this->columnCount(root), root);
         emit dataChanged(start, end);
         break;
      }
   }
}
void ConditionListModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   if (stub->formType != dovah::form_type::cell)
      return;
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         if (is_just_flagged) {
            //
            // TODO: Do we want to even display cells that were flagged as deleted?
            //
         }
         this->beginRemoveRows(QModelIndex(), i, i);
         list.remove(i);
         this->endRemoveRows();
         break;
      }
   }
}
void ConditionListModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         item->update();
         auto index = this->index(i, 1, QModelIndex());
         emit dataChanged(index, index);
         return;
      }
   }
}

QModelIndex ConditionListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex ConditionListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int ConditionListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int ConditionListModel::columnCount(const QModelIndex& item) const {
   return 6;
}
Qt::ItemFlags ConditionListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}
QVariant ConditionListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item    = (item_type*)index.internalPointer();
   auto column  = index.column();
   bool edited  = item->is_active;
   bool deleted = item->stub->is_deleted();
   switch (column) {
      case 0: // editor ID
         switch (role) {
            case Qt::DisplayRole:
               {
                  QString text = item->editorID;
                  if (text.isEmpty())
                     text = tr("Unnamed Cell", "cell view cell list");
                  if (deleted && role == Qt::DisplayRole)
                     text += tr(" * ", "edited form ID marker");
                  return text;
               }
            case Qt::FontRole:
               if (item->editorID.isEmpty()) {
                  auto font = QFont();
                  font.setItalic(true);
                  return font;
               }
               break;
         }
         break;
      case 1: // form ID
         switch (role) {
            case Qt::DisplayRole:
               return QString::asprintf("%08X", item->formID) + ((edited || deleted) ? tr(" * ", "edited form ID marker") : "") + (deleted ? tr("D", "deleted form ID marker") : "");
            case Qt::ForegroundRole:
               if (item->is_injected)
                  return QColor::fromRgb(0x309000);
               break;
         }
         break;
      case 2: // grid X
         switch (role) {
            case Qt::DisplayRole:
               return item->gridX;
         }
         break;
      case 3: // grid Y
         switch (role) {
            case Qt::DisplayRole:
               return item->gridY;
         }
         break;
   }
   return QVariant();
}
//
QVariant ConditionListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case ColumnTarget:   return tr("Target",    "condition list");
            case ColumnFunction: return tr("Function",  "condition list");
            case ColumnArgs:     return tr("Arguments", "condition list");
            case ColumnOperator: return tr("Operator",  "condition list");
            case ColumnOperand:  return tr("Operand",   "condition list");
            case ColumnUsesOr:   return tr("",          "condition list (uses-OR col header)");
         }
         break;
   }
   return QVariant();
}

void ConditionListModel::insertItem(const dovah::form_stub* stub, bool queued) {
   if (!stub)
      return;
   auto item = new item_type(stub);
   if (queued) {
      this->queued_additions.push_back(item);
   } else {
      auto i = this->children.size();
      this->beginInsertRows(QModelIndex(), i, i);
      this->children.push_back(item);
      this->endInsertRows();
   }
}

void ConditionListModel::clear() {
   this->beginResetModel();
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->endResetModel();
}
#pragma endregion

#pragma region ConditionList
ConditionList::ConditionList(QWidget* parent) : QTableView(parent) {
   this->setModel(new model_type);
   this->verticalHeader()->setDefaultSectionSize(0);
   this->sortByColumn(0, Qt::AscendingOrder);
   this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
   //
   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->resizeSection(1, metrics.boundingRect("00000000").width() * 1.5F + 4);
   header->resizeSection(2, metrics.boundingRect("000").width() * 1.5F + 4);
   header->resizeSection(3, metrics.boundingRect("000").width() * 1.5F + 4);
   header->setSectionResizeMode(0, QHeaderView::Stretch);
   header->setSectionResizeMode(1, QHeaderView::Interactive);
   header->setSectionResizeMode(2, QHeaderView::Interactive);
   header->setSectionResizeMode(3, QHeaderView::Interactive);
};
#pragma endregion