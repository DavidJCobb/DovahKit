#include "DKFormListPaneModel.h"
#include <QMimeData>
#include "../../helpers/qt/strings.h"
#include "../../editor/core.h"
#include "../../editor/helpers/form_identifiers_to_string.h"
#include "../../editor/helpers/form_stub_drag_drop.h"

#pragma region Item
DKFormListPaneModel::Item::Item(dovah::form_stub* stub) {
   this->stub = stub;
   this->updateFromStub();
}
void DKFormListPaneModel::Item::updateFromStub() {
   bool is_reference;
   auto stub = this->stub;
   if (stub) {
      this->editorID  = stub->get_editor_id();
      this->signature = editor_helpers::form_signature_to_string(stub);
      is_reference = dovah::form_type_is_reference(stub->form_type);
   } else {
      this->editorID.clear();
      this->signature = cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(dovah::form_type::none).signature);
      is_reference = false;
   }
   //
   //
   if (is_reference && this->editorID.isEmpty()) {
      const dovah::form_stub* cell  = nullptr;
      const dovah::form_stub* world = nullptr;
      //
      if (auto* parent = stub->get_parent_form()) {
         if (parent->form_type == dovah::form_type::cell) {
            cell = parent;
            //
            parent = cell->get_parent_form();
            if (parent->form_type == dovah::form_type::worldspace)
               world = parent;
         }
      }
      //
      if (cell) {
         const char* name = cell->get_editor_id();
         if (world) {
            if (name && name[0]) {
               this->editorID = QString("%1 in %2 in %3")
                  .arg(editor_helpers::form_identifiers_to_string(stub))
                  .arg(editor_helpers::form_identifiers_to_string(cell))
                  .arg(editor_helpers::form_identifiers_to_string(world));
            } else {
               this->editorID = QString("%1 in cell (%2, %3) in %4").arg(editor_helpers::form_identifiers_to_string(stub));
               //
               int32_t x;
               int32_t y;
               if (cell->get_grid_coordinates(x, y)) {
                  this->editorID = this->editorID.arg(x).arg(y);
               } else {
                  this->editorID = this->editorID.arg("?").arg("?");
               }
               this->editorID = this->editorID.arg(cell->formID);
            }
            this->editorID = this->editorID
               .arg(editor_helpers::form_identifiers_to_string(world));
         } else {
            this->editorID = QString("%1 in %2")
               .arg(editor_helpers::form_identifiers_to_string(stub))
               .arg(editor_helpers::form_identifiers_to_string(cell));
         }
      }
   }
}
#pragma endregion

DKFormListPaneModel::DKFormListPaneModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,   this, &DKFormListPaneModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,         this, &DKFormListPaneModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,    this, &DKFormListPaneModel::clear);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &DKFormListPaneModel::formsRenumberedEnMasse);
}

void DKFormListPaneModel::_addStub(dovah::form_stub* stub, bool queued) {
   if (!stub && !this->allow_gaps)
      return;
   if (stub && !this->allowed_form_types.isEmpty()) {
      if (!this->allowed_form_types.contains(stub->form_type))
         return;
   }
   auto item = new Item(stub);
   if (queued) {
      this->queued_additions.push_back(item);
   } else {
      auto first_inserted = this->children.size();
      auto last_inserted = first_inserted;
      this->beginInsertRows(QModelIndex(), first_inserted, last_inserted); // we're not passing the count, we're passing the index of the last row. how annoying.
      this->children.push_back(item);
      this->endInsertRows();
   }
}
void DKFormListPaneModel::_removeStub(Item* item) {
   auto index = this->children.indexOf(item);
   this->removeStub(index);
}
void DKFormListPaneModel::_updateStub(Item* item) {
   item->updateFromStub();
   //
   auto i     = this->children.indexOf(item);
   auto root  = QModelIndex();
   auto start = this->index(i, 0, root);
   auto end   = this->index(i, this->columnCount(root), root);
   emit dataChanged(start, end);
}
void DKFormListPaneModel::_pruneItems(std::function<bool(const Item&)> functor) {
   bool any_removed = false;
   for (auto*& item : this->children) {
      if (!item) {
         any_removed = true;
         continue;
      }
      if (functor(*item)) {
         delete item;
         item = nullptr;
         any_removed = true;
      }
   }
   //
   // The model system makes pruning the list a pain in the neck...
   //
   if (!any_removed)
      return;
   QModelIndex dummy;
   int size = this->children.size();
   for (int i = 0; i < size; ++i) {
      auto* item = this->children[i];
      if (!item) {
         this->beginRemoveRows(dummy, i, i);
         this->children.remove(i);
         --size;
         --i;
         this->endRemoveRows();
      }
   }
}

QVector<dovah::form_stub*> DKFormListPaneModel::stubs() const noexcept {
   QVector<dovah::form_stub*> s;
   s.reserve(this->children.size());
   for (auto* item : this->children)
      s.push_back(item->stub);
   return s;
}


#pragma region Editor core hooks
void DKFormListPaneModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   auto& list = this->children;
   auto  size = list.size();
   for (int i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub)
         this->_removeStub(item);
   }
}
void DKFormListPaneModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         item->updateFromStub();
         auto index = this->index(i, 2, QModelIndex());
         emit dataChanged(index, index);
         return;
      }
   }
}
void DKFormListPaneModel::formsRenumberedEnMasse() {
   //
   // We don't store enough information to check which list items have had their 
   // form IDs changed, so just blindly update the form IDs for all list items.
   //
   QModelIndex upper_left  = this->index(0, 2, QModelIndex());
   QModelIndex lower_right = this->index(this->children.size() - 1, 2, QModelIndex());
   emit dataChanged(upper_left, lower_right);
}
#pragma endregion

#pragma region QAbstractItemModel overrides
QModelIndex DKFormListPaneModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   Item* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex DKFormListPaneModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int DKFormListPaneModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int DKFormListPaneModel::columnCount(const QModelIndex& item) const {
   return 3;
}
Qt::ItemFlags DKFormListPaneModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::ItemIsDropEnabled;
   return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}
QVariant DKFormListPaneModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item   = (Item*)index.internalPointer();
   auto column = index.column();
   switch (column) {
      case ColumnType: // signature
         switch (role) {
            case Qt::DisplayRole:
               return item->signature;
         }
         break;
      case ColumnName: // editor ID or parent cell information
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               return item->editorID;
         }
         break;
      case ColumnFormID: // form ID
         switch (role) {
            case Qt::DisplayRole:
               return QString("%1").arg(item->stub ? item->stub->formID : 0, 8, 16, QChar('0')).toUpper();
         }
         break;
   }
   return QVariant();
}
QVariant DKFormListPaneModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal) {
      if (this->show_indices && role == Qt::DisplayRole)
         return section;
      return QVariant();
   }
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case ColumnType:   return tr("Type", "FormList listview");
            case ColumnName:   return tr("Name", "FormList listview");
            case ColumnFormID: return tr("Form ID", "FormList listview");
         }
         break;
   }
   return QVariant();
}
inline const DKFormListPaneModel::Item* DKFormListPaneModel::row(int rowIndex) const noexcept {
   return this->children.value(rowIndex);
}

bool DKFormListPaneModel::moveRows(const QModelIndex& from_parent, int first_row_index, int count, const QModelIndex& to_parent, int to_position) {
   //
   // NOTE: This API's design is rather unintuitive.
   // 
   // You probably assume that these arguments have the following meanings:
   //
   //  - (first_row_index), or (sourceRow) in Qt's docs, is the index of the first row we want 
   //    to move.
   //
   //  - (count) is the number of elements we want to move, including the first one (so it must 
   //    be at least one).
   //
   //  - (to_position), or (destinationChild) in Qt's docs, is the index to which we want to 
   //    move the first of the rows we're moving; the first row should have this index after 
   //    the move operation is complete.
   // 
   // Unfortunately, this isn't always the case. If you're moving items downward within the 
   // same parent, then the (to_position) argument is actually the index *above* which the 
   // *last* of the rows-to-be-moved should be placed.
   //
   auto& list = this->children;
   auto  size = list.size();
   //
   // Correctness checks:
   //
   if (first_row_index < 0 || count <= 0 || to_position < 0) // don't move anything to/from before the start of the table
      return false;
   if (to_position > size) // don't move anything past the end of the table
      return false;
   auto last_to_move = first_row_index + count - 1;
   if (last_to_move >= size) // don't move anything past the end of the table
      return false;
   //
   // Let Qt run its own correctness checks and then run its preparations. Among other things, 
   // this should check to make sure that the target position isn't in the middle of the range of 
   // rows you're moving.
   //
   if (!beginMoveRows(from_parent, first_row_index, last_to_move, to_parent, to_position))
      return false;
   //
   // If we're moving elements up, then we can just move the element at (last_to_move) repeatedly. 
   // See, when we move the furthest-down of the elements to be moved, then it'll displace all of 
   // its previous siblings downward by one, such that moving the same index gets the next element 
   // to be moved.
   //
   // If we're moving elements down, then we can just move the element at (first_row_index) over 
   // and over. When we move it down, we displace all of its next-siblings upward, such that moving
   // the same index gets the next element to be moved.
   //
   int target = last_to_move;
   if (to_position >= first_row_index) { // are we moving elements down?
      target = first_row_index;
      //
      // We also have to decrement (to_position) only when moving down, to make up for moveRows 
      // being completely cursed.
      //
      // NOT decrementing this means that you'll move items down one too far. ALWAYS decrementing 
      // this means that trying to move an item from index 1 to index 0 will crash, unless you 
      // just straight-up don't even allow that, which is QListWidget's approach.
      //
      --to_position;
   }
   while (count--)
      list.move(target, to_position);
   //
   // And we're done!
   //
   endMoveRows();
   return true;
}
#pragma endregion

#pragma region Drag-and-drop
bool DKFormListPaneModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
   if (!data->hasFormat(editor_helpers::form_stub_array_mime_type))
      return false;
   return true;
}
bool DKFormListPaneModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
   if (!canDropMimeData(data, action, row, column, parent))
      return false;
   if (action == Qt::IgnoreAction)
      return true;
   if (row == -1) {
      if (parent.isValid())
         row = parent.row();
      else
         row = this->children.size();
   }
   //
   auto dropped_stubs = editor_helpers::form_stubs_from_mime_data(*data);
   QVector<Item*> queued;
   queued.reserve(dropped_stubs.size());
   for (auto* stub : dropped_stubs) {
      if (!stub)
         continue;
      if (!this->allowed_form_types.isEmpty()) {
         if (!this->allowed_form_types.contains(stub->form_type))
            continue;
      }
      queued.push_back(new Item(stub));
   }
   auto first_inserted = row;
   auto last_inserted  = first_inserted + queued.size() - 1;
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted); // we're not passing the count, we're passing the index of the last row. how annoying.
   this->children.reserve(this->children.size() + queued.size());
   for (int i = 0; i < queued.size(); ++i) {
      auto* item = queued[i];
      this->children.insert(row + i, item);
   }
   this->endInsertRows();
   return true;
}
QStringList DKFormListPaneModel::mimeTypes() const {
   return QStringList(QString(editor_helpers::form_stub_array_mime_type));
}
Qt::DropActions DKFormListPaneModel::supportedDropActions() const {
   return Qt::CopyAction;
}
#pragma endregion

void DKFormListPaneModel::clear() {
   if (this->children.empty())
      return;
   this->beginRemoveRows(QModelIndex(), 0, this->children.size() - 1); // don't use beginResetModel; Qt documentation doesn't seem to mention this anywhere but it breaks hidden columns in table views
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->endRemoveRows();
}
void DKFormListPaneModel::moveStubs(QModelIndexList indices, int down) {
   if (indices.isEmpty())
      return;
   auto size = this->children.size();
   for (auto& index : indices) {
      auto i = index.row();
      if (i >= size)
         continue;
      int to = i + down;
      if (to < 0)
         to = 0;
      else if (to >= size)
         to = size - 1;
      if (down > 0)
         //
         // When moving rows up within the same parent, or across parents, the "destination index" 
         // is the index they will be placed at. However, when moving rows up within the same 
         // parent, the "destination index" is the index that they will be placed BEFORE. In fact, 
         // if you're moving multiple rows, then the "destination index" should be index AFTER the 
         // index that you want the LAST moved element placed at.
         //
         // I have no idea what Qt was trying to achieve with this API design, but it's pretty 
         // damn cursed.
         //
         ++to;
      this->moveRow(QModelIndex(), i, QModelIndex(), to);
   }
}
void DKFormListPaneModel::removeStub(int index) {
   auto& list = this->children;
   if (index < 0 || index >= list.size())
      return;
   this->beginRemoveRows(QModelIndex(), index, index);
   list.removeAt(index);
   this->endRemoveRows();
}
void DKFormListPaneModel::removeStubs(QVector<int> indices) {
   if (indices.isEmpty())
      return;
   QVector<Item*> keep;
   QModelIndex parent_index;
   //
   auto& list = this->children;
   auto  size = list.size();
   for (int i = 0; i < size; ++i) {
      if (indices.contains(i)) {
         this->beginRemoveRows(parent_index, i, i);
         delete list[i];
         this->endRemoveRows();
      } else {
         keep.push_back(list[i]);
      }
   }
   keep.swap(list);
}
void DKFormListPaneModel::removeStubs(QModelIndexList l) {
   if (l.isEmpty())
      return;
   QVector<int> indices;
   for (auto& i : l)
      indices.push_back(i.row());
   this->removeStubs(indices);
}

#pragma region Property setters
void DKFormListPaneModel::setAllowedFormTypes(QVector<form_type> l) {
   this->allowed_form_types = l;
   if (l.isEmpty())
      return;
   this->_pruneItems([this](const Item& item) {
      if (!item.stub)
         return !this->allow_gaps;
      return !this->allowed_form_types.contains(item.stub->form_type);
   });
}
void DKFormListPaneModel::setAllowGaps(bool g) {
   this->allow_gaps = g;
   if (g)
      return;
   this->_pruneItems([this](const Item& item) {
      return !item.stub;
   });
}
void DKFormListPaneModel::setShowIndices(bool s) {
   if (s == this->show_indices)
      return;
   this->show_indices = s;
   emit headerDataChanged(Qt::Vertical, 0, this->children.size() - 1);
}
#pragma endregion