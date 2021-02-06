#include "formlist_listview.h"
#include <QHeaderView>
#include <QKeyEvent>
#include <QMimeData>
#include "../../../helpers/qt/strings.h"
#include "../../../editor/core.h"
#include "../../../editor/open_window_for_form.h"

#pragma region FormListListviewModel
FormListListviewModelItem::FormListListviewModelItem(dovah::form_stub* stub) {
   this->stub = stub;
   this->updateFromStub();
}
void FormListListviewModelItem::updateFromStub() {
   bool is_reference;
   auto stub = this->stub;
   if (stub) {
      this->editorID = stub->get_editor_id();
      //
      uint32_t signature = dovah::form_type_info::lookup(stub->formType).signature;
      this->signature = cobb::qt::four_cc_to_string(signature);
      is_reference = dovah::form_type_info::form_type_is_reference(stub->formType);
   } else {
      this->editorID.clear();
      this->signature = cobb::qt::four_cc_to_string(dovah::form_types[dovah::form_type::none].signature);
      is_reference = false;
   }
   //
   //
   if (is_reference && this->editorID.isEmpty()) {
      const dovah::form_stub* cell  = nullptr;
      const dovah::form_stub* world = nullptr;
      //
      if (auto* parent = stub->get_parent_form()) {
         if (parent->formType == dovah::form_type::cell) {
            cell = parent;
            //
            parent = cell->get_parent_form();
            if (parent->formType == dovah::form_type::worldspace)
               world = parent;
         }
      }
      //
      if (cell) {
         const char* name = cell->get_editor_id();
         if (world) {
            if (name && name[0]) {
               this->editorID = QString("[REFR:%1] in [CELL:%2]%3 in [WRLD:%4]%5")
                  .arg(QString("%1").arg(stub->formID, 8, 16, QChar('0')).toUpper())
                  .arg(QString("%1").arg(cell->formID, 8, 16, QChar('0')).toUpper())
                  .arg(name);
            } else {
               this->editorID = QString("[REFR:%1] in cell (%2, %3) in [WRLD:%4]%5");
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
               .arg(QString("%1").arg(world->formID, 8, 16, QChar('0')).toUpper())
               .arg(world->get_editor_id());
         } else {
            this->editorID = QString("[REFR:%1] in [CELL:%2]%3")
               .arg(QString("%1").arg(stub->formID, 8, 16, QChar('0')).toUpper())
               .arg(QString("%1").arg(cell->formID, 8, 16, QChar('0')).toUpper())
               .arg(name ? name : "");
         }
      }
   }
}

FormListListviewModel::FormListListviewModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,   this, &FormListListviewModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,         this, &FormListListviewModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,    this, &FormListListviewModel::clear);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &FormListListviewModel::formsRenumberedEnMasse);
}

void FormListListviewModel::removeStub(item_type* item) {
   auto index = this->children.indexOf(item);
   this->removeStub(index);
}
void FormListListviewModel::updateStub(item_type* item) {
   item->updateFromStub();
   //
   auto i     = this->children.indexOf(item);
   auto root  = QModelIndex();
   auto start = this->index(i, 0, root);
   auto end   = this->index(i, this->columnCount(root), root);
   emit dataChanged(start, end);
}
void FormListListviewModel::_pruneItems(std::function<bool(const item_type&)> functor) {
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

void FormListListviewModel::addStub(dovah::form_stub* stub, bool queued) {
   if (!stub && !this->allow_gaps)
      return;
   if (stub && !this->allowed_form_types.isEmpty()) {
      if (!this->allowed_form_types.contains(stub->formType))
         return;
   }
   auto item = new item_type(stub);
   if (queued) {
      this->queued_additions.push_back(item);
   } else {
      auto first_inserted = this->children.size();
      auto last_inserted  = first_inserted;
      this->beginInsertRows(QModelIndex(), first_inserted, last_inserted); // we're not passing the count, we're passing the index of the last row. how annoying.
      this->children.push_back(item);
      this->endInsertRows();
   }
}
void FormListListviewModel::moveStubs(QModelIndexList indices, int down) {
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
void FormListListviewModel::removeStub(int index) {
   auto& list = this->children;
   if (index < 0 || index >= list.size())
      return;
   this->beginRemoveRows(QModelIndex(), index, index);
   list.removeAt(index);
   this->endRemoveRows();
}
void FormListListviewModel::removeStubs(QVector<int> indices) {
   if (indices.isEmpty())
      return;
   QVector<item_type*> keep;
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
void FormListListviewModel::removeStubs(QModelIndexList l) {
   if (l.isEmpty())
      return;
   QVector<int> indices;
   for (auto& i : l)
      indices.push_back(i.row());
   this->removeStubs(indices);
}
void FormListListviewModel::setAllowedFormTypes(QVector<form_type_t> l) {
   this->allowed_form_types = l;
   if (l.isEmpty())
      return;
   this->_pruneItems([this](const item_type& item) {
      if (!item.stub)
         return !this->allow_gaps;
      return !this->allowed_form_types.contains(item.stub->formType);
   });
}
void FormListListviewModel::setAllowGaps(bool g) {
   this->allow_gaps = g;
   if (g)
      return;
   this->_pruneItems([this](const item_type& item) {
      return !item.stub;
   });
}
void FormListListviewModel::setShowIndices(bool s) {
   if (s == this->show_indices)
      return;
   this->show_indices = s;
   emit headerDataChanged(Qt::Vertical, 0, this->children.size() - 1);
}
QVector<dovah::form_stub*> FormListListviewModel::stubs() const noexcept {
   QVector<dovah::form_stub*> s;
   s.reserve(this->children.size());
   for (auto* item : this->children)
      s.push_back(item->stub);
   return s;
}

void FormListListviewModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   auto& list = this->children;
   auto  size = list.size();
   for (int i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub)
         this->removeStub(item);
   }
}
void FormListListviewModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
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
void FormListListviewModel::formsRenumberedEnMasse() {
   //
   // We don't store enough information to check which list items have had their 
   // form IDs changed, so just blindly update the form IDs for all list items.
   //
   QModelIndex upper_left  = this->index(0, 2, QModelIndex());
   QModelIndex lower_right = this->index(this->children.size() - 1, 2, QModelIndex());
   emit dataChanged(upper_left, lower_right);
}
//
QModelIndex FormListListviewModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex FormListListviewModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int FormListListviewModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int FormListListviewModel::columnCount(const QModelIndex& item) const {
   return 3;
}
Qt::ItemFlags FormListListviewModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::ItemIsDropEnabled;
   return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}
QVariant FormListListviewModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item   = (item_type*)index.internalPointer();
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
inline const FormListListviewModel::item_type* FormListListviewModel::row(int rowIndex) const noexcept {
   return this->children.value(rowIndex);
}
//
bool FormListListviewModel::moveRows(const QModelIndex& from_parent, int first_row_index, int count, const QModelIndex& to_parent, int to_position) {
   //
   // NOTE: This API's design is cursed. It's unintuitive, and not even in a manner that has 
   // some obvious utility to the implementors. In fact, it's *harder* to implement than it 
   // would be had it been designed the obvious way.
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
   // But you see, Qt's developers had a much better idea. They decided that all of the above 
   // should be true EXCEPT when moving items downward within the same parent. In *that* case, 
   // the (to_position) argument is actually the index *above* which the *last* of the rows-to-
   // be-moved should be placed. This is super unintuitive, but that's fine, since it's super 
   // rare to move rows downward within the same parent. Never happens.
   //
   // On top of that, as far as I can see, Qt doesn't even have its own implementation of this 
   // function for QTableWidget, so the only viable implementation is the one on QListWidget, 
   // which may literally be broken per the notes below.
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
      // being completely cursed. As of this writing, QListWidget's official implementation for 
      // moveRows decrements the argument unconditionally, so that's probably broken. This API 
      // is so cursed that it may have confused its own designers.
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
//
QVariant FormListListviewModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal) {
      if (this->show_indices && role == Qt::DisplayRole)
         return section;
      return QVariant();
   }
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case ColumnType:   return tr("Type",    "FormList listview");
            case ColumnName:   return tr("Name",    "FormList listview");
            case ColumnFormID: return tr("Form ID", "FormList listview");
         }
         break;
   }
   return QVariant();
}
bool FormListListviewModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
   if (!data->hasFormat("application/dovah-kit.form-id-array"))
      return false;
   return true;
}
bool FormListListviewModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
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
   QByteArray  bytes = data->data("application/dovah-kit.form-id-array");
   QDataStream stream(&bytes, QIODevice::ReadOnly);
   QVector<uint32_t> formIDs;
   while (!stream.atEnd()) {
      uint32_t id;
      uint8_t  delim;
      stream.readRawData((char*)&id, 4);
      stream.readRawData((char*)&delim, 1);
      assert(!delim);
      formIDs.push_back(id);
   }
   //
   auto& editor = DovahKitCore::get();
   QVector<item_type*> queued;
   queued.reserve(formIDs.size());
   for (auto id : formIDs) {
      auto* stub = editor.get_form(id);
      if (stub) {
         if (!this->allowed_form_types.isEmpty()) {
            if (!this->allowed_form_types.contains(stub->formType))
               continue;
         }
         queued.push_back(new item_type(stub));
      }
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
QStringList FormListListviewModel::mimeTypes() const {
   return QStringList(QString("application/dovah-kit.form-id-array"));
}
Qt::DropActions FormListListviewModel::supportedDropActions() const {
   return Qt::CopyAction;
}

void FormListListviewModel::clear() {
   this->beginResetModel();
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->endResetModel();
}
#pragma endregion

#pragma region FormListListview
FormListListview::FormListListview(QWidget* parent) : QTableView(parent) {
   this->setModel(new model_type(this));
   this->setSelectionBehavior(QAbstractItemView::SelectRows);
   this->setSelectionMode(QAbstractItemView::ExtendedSelection);
   //
   this->setAcceptDrops(true);
   this->setDragDropOverwriteMode(false);
   //
   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->resizeSection(model_type::ColumnType,   metrics.boundingRect("XMMX").width() * 1.5F + 4);
   header->resizeSection(model_type::ColumnFormID, 4);
   header->setSectionResizeMode(model_type::ColumnType,   QHeaderView::Interactive);
   header->setSectionResizeMode(model_type::ColumnName,   QHeaderView::Stretch);
   header->setSectionResizeMode(model_type::ColumnFormID, QHeaderView::Interactive);
   header->setStretchLastSection(false);
   this->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   //
   QObject::connect(this, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      if (!index.isValid())
         return;
      auto* model = this->model();
      auto  data  = (model_item_type*)index.internalPointer();
      if (data && data->stub)
         open_edit_dialog_for_form(data->stub, this);
   });
}
void FormListListview::addStub(dovah::form_stub* stub) {
   auto* model = (model_type*)this->model();
   if (!model)
      return;
   model->addStub(stub);
}
void FormListListview::clear() {
   auto* model = (model_type*)this->model();
   if (!model)
      return;
   model->clear();
}
void FormListListview::reserve(size_t i) {
   auto* model = (model_type*)this->model();
   if (!model)
      return;
   model->reserve(i);
}
void FormListListview::setTarget(const dovah::form_stub* stub) {
   this->target = stub;
}
QVector<dovah::form_stub*> FormListListview::stubs() const noexcept {
   auto* model = (model_type*)this->model();
   if (!model)
      return QVector<dovah::form_stub*>();
   return model->stubs();
}

void FormListListview::moveSelected(int down) {
   auto* model = (model_type*)this->model();
   auto* sm    = this->selectionModel();
   if (!model || !sm)
      return;
   model->moveStubs(sm->selectedRows(), down);
}
void FormListListview::removeSelected() {
   auto* model = (model_type*)this->model();
   auto* sm    = this->selectionModel();
   if (!model || !sm)
      return;
   model->removeStubs(sm->selectedRows());
}

void FormListListview::import(const std::vector<dovah::form_reference_t>& list) {
   this->clear();
   this->reserve(list.size());
   for (auto& ref : list)
      this->addStub(ref.get_form_stub());
}
void FormListListview::commit(std::vector<dovah::form_reference_t>& list, dovah::loaded_forms::Form& owner) {
   auto   stubs = this->stubs();
   size_t i     = 0;
   size_t size  = stubs.size();
   if (list.size() < size)
      list.resize(size);
   for (; i < size; ++i)
      list[i].set(owner, stubs[i]);
   //
   // Delete excess elements, if any were removed:
   //
   auto s = list.size();
   if (s != size) {
      for (; i < s; ++i)
         list[i].set(owner, nullptr);
      list.resize(size);
   }
}

void FormListListview::keyPressEvent(QKeyEvent* event) {
   if (event->matches(QKeySequence::Delete)) {
      this->removeSelected();
   }
}
#pragma endregion