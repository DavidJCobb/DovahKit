#include "DKFormListPaneModel.h"
#include <QMimeData>
#include "helpers/qt/strings.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "editor/core.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/helpers/form_stub_drag_drop.h"

#pragma region Item
DKFormListPaneModel::Item::Item() {
   this->columnText.resize(cached_builtin_column_count);
}
DKFormListPaneModel::Item::Item(dovah::form_stub* stub) : Item() {
   this->stub = stub;
}

QString DKFormListPaneModel::Item::computeEditorID() const {
   auto* stub = this->stub;
   if (!stub)
      return "";

   QString result = stub->get_editor_id();
   if (!result.isEmpty())
      return result;

   //
   // Handle nameless refs and cells: a nameless ref can be identified based on the 
   // cell it's in, and a nameless cell can be identified based on the world it's in.
   //

   const dovah::form_stub* ref   = nullptr;
   const dovah::form_stub* cell  = nullptr;
   const dovah::form_stub* world = nullptr;
   if (stub->form_type == dovah::form_type::cell) {
      cell = stub;
      if (auto* parent = stub->get_parent_form()) {
         if (parent->form_type == dovah::form_type::worldspace) [[likely]] {
            world = parent;
         }
      }
   } else if (dovah::form_type_is_reference(stub->form_type)) {
      ref = stub;
      if (auto* parent = stub->get_parent_form()) [[likely]] {
         if (parent->form_type == dovah::form_type::cell) [[likely]] {
            cell   = parent;
            parent = cell->get_parent_form();
            if (parent) {
               if (parent->form_type == dovah::form_type::worldspace) [[likely]] {
                  world = parent;
               }
            }
         }
      }
   } else {
      return "";
   }

   if (!cell) {
      assert(ref != nullptr);
      return editor_helpers::form_identifiers_to_string(ref);
   }
   QString cell_text = cell->get_editor_id();
   if (cell_text.isEmpty()) {
      if (world) {
         QString world_text = world->get_editor_id();
         if (world_text.isEmpty()) [[unlikely]] {
            world_text = editor_helpers::form_identifiers_to_string(world);
         }

         cell_text = QString("(%1, %2) in %3");

         int32_t x;
         int32_t y;
         if (cell->get_grid_coordinates(x, y)) [[likely]] {
            cell_text = cell_text.arg(x).arg(y);
         } else {
            cell_text = cell_text.arg("?").arg("?");
         }
         cell_text = cell_text.arg(world_text);
      } else [[unlikely]] {
         //
         // It's a nameless interior cell. This generally shouldn't happen.
         //
         if (ref) {
            cell_text = editor_helpers::form_identifiers_to_string(cell);
         }
      }
   }
   if (!ref) {
      return QString("Cell %1").arg(cell_text);
   }
   return QString("%1 in cell %2").arg(editor_helpers::form_identifiers_to_string(ref)).arg(cell_text);
}
QString DKFormListPaneModel::Item::computeSignature() const {
   if (!this->stub)
      return cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(dovah::form_type::none).signature);
   return editor_helpers::form_signature_to_string(this->stub);
}

QString DKFormListPaneModel::Item::getCachedEditorID() const {
   return this->columnText[1];
}
QString DKFormListPaneModel::Item::getCachedSignature() const {
   return this->columnText[0];
}
#pragma endregion

DKFormListPaneModel::DKFormListPaneModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,   this, &DKFormListPaneModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,         this, &DKFormListPaneModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::formModified,           this, &DKFormListPaneModel::formModified);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,    this, &DKFormListPaneModel::clear);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &DKFormListPaneModel::formsRenumberedEnMasse);
}

bool DKFormListPaneModel::_allowsForm(dovah::form_stub& stub) {
   if (!this->allowed_form_types.isEmpty()) {
      if (!this->allowed_form_types.contains(stub.form_type))
         return false;
   }
   if (this->_custom_filter)
      if (!this->_custom_filter->form_matches(stub))
         return false;
   return true;
}
void DKFormListPaneModel::_emitRowChanged(size_t row) {
   size_t column_count = ColumnCount;
   column_count += this->extra_columns.list.size();

   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count - 1, {});
   emit dataChanged(tl, br);
}
void DKFormListPaneModel::_recacheNewlyAppendedExtraColumn(Item& item, extra_column_handler& handler) {
   QString str;
   if (item.stub) {
      if (std::holds_alternative<extra_column_handler_by_data>(handler)) {
         auto loaded = item.stub->load();
         if (loaded)
            str = std::get<extra_column_handler_by_data>(handler)(*loaded);
      } else {
         str = std::get<extra_column_handler_by_stub>(handler)(*item.stub);
      }
   }
   item.columnText.push_back(str);
}
void DKFormListPaneModel::_recacheItemText(Item& item) {
   auto& extra = this->extra_columns.list;

   bool needs_loaded = this->extra_columns.any_getters_take_loaded_form;

   dovah::loaded_form_ptr<dovah::loaded_forms::Form> loaded;
   if (item.stub && needs_loaded) {
      loaded = item.stub->load();
   }

   item.columnText.resize(Item::cached_builtin_column_count + extra.size());

   QString editor_id;
   {
      bool done = false;
      if (item.stub && item.stub->editorID.empty() && dovah::form_type_is_reference(item.stub->form_type)) {
         if (this->nameless_refs_show_base_editor_id) {
            auto* base = dovah::form_stub_helpers::get_base_form(item.stub);
            if (base && !base->editorID.empty()) {
               editor_id = QString::fromStdString(base->editorID);
               done = true;
            }
         }
      }
      if (!done) {
         editor_id = item.computeEditorID();
      }
   }

   item.columnText[Item::cache_index_for_builtin_column(Column::Type).value()] = item.computeSignature();
   item.columnText[Item::cache_index_for_builtin_column(Column::Name).value()] = editor_id;

   for (size_t i = 0; i < extra.size(); ++i) {
      auto& dst     = item.columnText[Item::cache_index_for_extra_column(i)];
      auto& handler = extra[i].handler;
      if (!item.stub) {
         dst.clear();
         continue;
      }
      if (needs_loaded && std::holds_alternative<extra_column_handler_by_data>(handler)) {
         if (loaded)
            dst = std::get<extra_column_handler_by_data>(handler)(*loaded);
         else
            dst = "";
      } else {
         dst = std::get<extra_column_handler_by_stub>(handler)(*item.stub);
      }
   }
}

void DKFormListPaneModel::_pruneItems(std::function<bool(const Item&)> functor) {
   QModelIndex dummy;
   size_t size = this->children.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = this->children[i];
      if (functor(*item)) {
         this->beginRemoveRows(dummy, i, i);
         this->children.remove(i);
         delete item;
         --size;
         --i;
         this->endRemoveRows();
      }
   }
}

void DKFormListPaneModel::_clear(bool silent) {
   if (this->children.empty())
      return;
   if (!silent) {
      this->beginRemoveRows({}, 0, this->children.size() - 1); // don't use beginResetModel; Qt documentation doesn't seem to mention this anywhere but it breaks hidden columns in table views
   }
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   if (!silent) {
      this->endRemoveRows();
   }
}

QVector<dovah::form_stub*> DKFormListPaneModel::stubs() const noexcept {
   QVector<dovah::form_stub*> s;
   s.reserve(this->children.size());
   for (auto* item : this->children)
      s.push_back(item->stub);
   return s;
}
dovah::form_stub* DKFormListPaneModel::getNthStub(size_t i) const noexcept {
   if (i >= this->children.size())
      return nullptr;
   return this->children[i]->stub;
}

#pragma region Editor core hooks
void DKFormListPaneModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   this->_pruneItems([stub](const Item& item) {
      return item.stub == stub;
   });
}
void DKFormListPaneModel::formModified(dovah::form_stub* stub) {
   bool has_extra_columns = false;
   if (!this->extra_columns.list.empty())
      has_extra_columns = true;

   auto&  list = this->children;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub != stub)
         continue;

      if (this->_custom_filter) {
         if (!this->_custom_filter->form_matches(*item->stub)) {
            this->beginRemoveRows({}, i, i);
            delete item;
            list.removeAt(i);
            --i;
            --size;
            this->endRemoveRows();
            continue;
         }
      }

      this->_recacheItemText(*item);
      if (has_extra_columns) {
         this->_emitRowChanged(i);
      } else {
         auto qmi = this->index(i, Column::Name, {});
         emit dataChanged(qmi, qmi);
      }
   }
}
void DKFormListPaneModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         this->_recacheItemText(*item);
         this->_emitRowChanged(i);
         return;
      }
   }
}
void DKFormListPaneModel::formsRenumberedEnMasse() {
   //
   // We don't store enough information to check which list items have had their 
   // form IDs changed, so just blindly update the form IDs for all list items.
   //
   auto column_count = this->columnCount({});
   QModelIndex upper_left  = this->index(0,                         column_count - 1, {});
   QModelIndex lower_right = this->index(this->children.size() - 1, column_count - 1, {});
   emit dataChanged(upper_left, lower_right);
}
#pragma endregion

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      QModelIndex DKFormListPaneModel::index(int row, int column, const QModelIndex& parent) const {
         if (!this->hasIndex(row, column, parent))
            return {};
         Item* childItem = this->children.value(row);
         if (childItem)
            return this->createIndex(row, column, childItem);
         return {};
      }
      QModelIndex DKFormListPaneModel::parent(const QModelIndex& index) const {
         return {};
      }
      int DKFormListPaneModel::rowCount(const QModelIndex& parent) const {
         if (parent.column() > 0)
            return 0;
         return this->children.size();
      }
      int DKFormListPaneModel::columnCount(const QModelIndex& item) const {
         return ColumnCount + this->extra_columns.list.size();
      }
   #pragma endregion
   #pragma region Data
      Qt::ItemFlags DKFormListPaneModel::flags(const QModelIndex& index) const {
         if (!index.isValid())
            return Qt::ItemFlag::ItemIsDropEnabled;
         return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
      }
      QVariant DKFormListPaneModel::data(const QModelIndex& index, int role) const {
         if (!index.isValid())
            return {};
         auto item   = (Item*)index.internalPointer();
         auto column = index.column();

         switch (column) {
            case Column::Type: // signature
               if (role == Qt::DisplayRole)
                  return item->getCachedSignature();
               return {};
            case Column::Name: // editor ID or parent cell information
               switch (role) {
                  case Qt::DisplayRole:
                  case Qt::ToolTipRole:
                     return item->getCachedEditorID();
               }
               return {};
            case Column::FormID: // form ID
               if (role == Qt::DisplayRole)
                  return editor_helpers::form_id_to_string(item->stub ? item->stub->formID : 0);
               return {};
         }
         if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
            column -= ColumnCount;
            if (column < this->extra_columns.list.size()) {
               return item->columnText[Item::cache_index_for_extra_column(column)];
            }
         }
         return {};
      }
      QVariant DKFormListPaneModel::headerData(int section, Qt::Orientation orientation, int role) const {
         if (orientation != Qt::Orientation::Horizontal) {
            if (this->show_indices && role == Qt::DisplayRole)
               return section;
            return {};
         }
         switch (role) {
            case Qt::DisplayRole:
               switch (section) {
                  case Column::Type:   return tr("Type", "FormList listview");
                  case Column::Name:   return tr("Name", "FormList listview");
                  case Column::FormID: return tr("Form ID", "FormList listview");
               }
               if (section >= ColumnCount) {
                  size_t i = section - ColumnCount;
                  if (i < this->extra_columns.list.size()) {
                     return this->extra_columns.list[i].header_text;
                  }
               }
               break;
         }
         return {};
      }
   #pragma endregion

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

   #pragma region Drag-and-drop
      bool DKFormListPaneModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
         if (!data->hasFormat(editor_helpers::form_stub_array_mime_type))
            return false;
         return true;
      }
      bool DKFormListPaneModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
         if (!this->canDropMimeData(data, action, row, column, parent))
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
         this->addStubs(dropped_stubs, row);
         return true;
      }
      QStringList DKFormListPaneModel::mimeTypes() const {
         return QStringList(QString(editor_helpers::form_stub_array_mime_type));
      }
      Qt::DropActions DKFormListPaneModel::supportedDropActions() const {
         return Qt::CopyAction;
      }
   #pragma endregion
#pragma endregion
               
#pragma region DKCustomFormFilterableModelMixin overrides
   /*virtual*/ void DKFormListPaneModel::recheck_custom_filter_for_all_forms() /*override*/ {
      if (!this->_custom_filter)
         return;
      auto&  list = this->children;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto* item = list[i];
         if (!item->stub)
            continue;

         if (!this->_custom_filter->form_matches(*item->stub)) {
            this->beginRemoveRows({}, i, i);
            list.removeAt(i);
            delete item;
            --i;
            --size;
         }
      }
   }
   /*virtual*/ void DKFormListPaneModel::recheck_custom_filter_for_form(dovah::form_stub& stub) /*override*/ {
      if (!this->_custom_filter)
         return;
      auto&  list = this->children;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto* item = list[i];
         if (item->stub != &stub)
            continue;

         if (!this->_custom_filter->form_matches(stub)) {
            this->beginRemoveRows({}, i, i);
            list.removeAt(i);
            delete item;
            --i;
            --size;
         }

         if (!this->allow_dupes)
            break;
      }
   }
#pragma endregion

void DKFormListPaneModel::addStub(dovah::form_stub* stub) {
   if (stub) {
      if (!this->allowed_form_types.isEmpty())
         if (!this->allowed_form_types.contains(stub->form_type))
            return;
   } else {
      if (!this->allow_gaps)
         return;
   }
   if (this->allow_dupes == false) {
      if (this->indexOfStub(stub) >= 0)
         return;
   }
   size_t at = this->children.size();
   this->beginInsertRows({}, at, at);
   auto* item = new Item;
   item->stub = stub;
   this->children.push_back(item);
   this->_recacheItemText(*item);
   this->endInsertRows();
}
void DKFormListPaneModel::addStubs(const std::vector<dovah::form_stub*>& src, int at) {
   QVector<Item*> queued;
   queued.reserve(src.size());

   for (size_t i = 0; i < src.size(); ++i) {
      auto* stub = src[i];
      if (!stub)
         continue;
      if (!this->_allowsForm(*stub))
         continue;
      if (this->allow_dupes == false) {
         bool dupe = false;
         for (size_t j = 0; j < i; ++j) {
            if (src[j] == stub) {
               dupe = true;
               break;
            }
         }
         if (dupe)
            continue;
         if (this->indexOfStub(stub) >= 0)
            continue;
      }
      queued.push_back(new Item(stub));
   }

   if (queued.empty())
      return;

   if (at < 0)
      at = this->children.size();

   auto first_inserted = at;
   auto last_inserted  = first_inserted + queued.size() - 1;
   this->beginInsertRows({}, first_inserted, last_inserted); // we're not passing the count, we're passing the index of the last row. how annoying.
   this->children.reserve(this->children.size() + queued.size());
   for (int i = 0; i < queued.size(); ++i) {
      auto* item = queued[i];
      this->children.insert(at + i, item);
      this->_recacheItemText(*item);
   }
   this->endInsertRows();
}
void DKFormListPaneModel::clear() {
   this->_clear();
}
int DKFormListPaneModel::indexOfStub(const dovah::form_stub* s) const {
   for (size_t i = 0; i < this->children.size(); ++i)
      if (this->children[i]->stub == s)
         return i;
   return -1;
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
      this->moveRow({}, i, {}, to);
   }
}
void DKFormListPaneModel::removeStub(int index) {
   auto& list = this->children;
   if (index < 0 || index >= list.size())
      return;
   this->beginRemoveRows({}, index, index);
   auto* item = list[index];
   list.removeAt(index);
   assert(item != nullptr);
   delete item;
   this->endRemoveRows();
}
void DKFormListPaneModel::removeStubs(QVector<int> indices) {
   if (indices.isEmpty())
      return;

   QVector<size_t> keep_indices;
   QModelIndex     parent_index;

   auto&  list = this->children;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i)
      if (!indices.contains(i))
         keep_indices.push_back(i);

   if (keep_indices.empty()) {
      //
      // We're keeping nothing -- clearing the whole list.
      //
      this->clear();
      return;
   }
   if (keep_indices.size() == size) [[unlikely]] {
      //
      // We're not removing anything. (Edge-case: the list of row indices contains 
      // only invalid row indices.)
      //
      return;
   }

   size_t last_row_to_keep = 0;
   size_t this_row_to_keep = 0;
   size_t count_deleted    = 0;
   for (size_t n = 0; n < keep_indices.size(); last_row_to_keep = this_row_to_keep, ++n) {
      this_row_to_keep = keep_indices[n] - count_deleted;
      if (this_row_to_keep == 0) {
         //
         // We're keeping the first item in the list; ergo there are no items before 
         // it to delete.
         //
         continue;
      }
      if (last_row_to_keep + 1 == this_row_to_keep) {
         //
         // Consecutive kept rows. Nothing to remove.
         //
         continue;
      }
      //
      // There are rows to remove, between the last row we kept and the current row 
      // we're keeping. Remove the rows between them.
      //
      size_t first_to_delete = last_row_to_keep + 1;
      size_t last_to_delete  = this_row_to_keep - 1;
      size_t count_to_delete = last_to_delete - first_to_delete + 1;

      this->beginRemoveRows(parent_index, first_to_delete, last_to_delete);
      for (size_t i = first_to_delete; i <= last_to_delete; ++i) {
         delete list[i];
      }
      list.remove(first_to_delete, count_to_delete);
      this->endRemoveRows();

      count_deleted    += count_to_delete;
      this_row_to_keep -= count_to_delete;
   }

   //
   // Check for any rows at the end of the list that need to be removed.
   //
   auto last_to_keep = keep_indices.back() - count_deleted;
   auto last_left    = size - count_deleted - 1;
   if (last_to_keep < last_left) {
      size_t first_to_delete = last_to_keep + 1;
      size_t count_to_delete = last_left - first_to_delete + 1;

      this->beginRemoveRows(parent_index, first_to_delete, last_left);
      for (size_t i = first_to_delete; i <= last_left; ++i) {
         delete list[i];
      }
      list.remove(first_to_delete, count_to_delete);
      this->endRemoveRows();
   }
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
void DKFormListPaneModel::setAllowDuplicates(bool v) {
   if (this->allow_dupes == v)
      return;
   this->allow_dupes = v;
   if (!v) {
      auto&  list = this->children;
      size_t size = list.size();
      for (size_t i = 0; i < size - 1; ++i) {
         auto* a = list[i]->stub;
         for (size_t j = i + 1; j < size; ++j) {
            auto* b = list[j]->stub;
            if (a == b) {
               this->beginRemoveRows({}, j, j);
               delete list[j];
               list.erase(list.begin() + j);
               --j;
               --size;
               this->endRemoveRows();
               continue;
            }
         }
      }
   }
}
void DKFormListPaneModel::setAllowedFormTypes(QVector<dovah::form_type> l) {
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
   if (g == this->allow_gaps)
      return;
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
void DKFormListPaneModel::setNamelessRefsShowBaseEditorID(bool v) {
   if (this->nameless_refs_show_base_editor_id == v)
      return;
   this->nameless_refs_show_base_editor_id = v;

   auto&        list = this->children;
   const size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (!item->stub->editorID.empty())
         continue;
      this->_recacheItemText(*item);
      auto qmi = this->index(i, Column::Name, {});
      emit dataChanged(qmi, qmi);
   }
}
#pragma endregion

#pragma region Extra columns
void DKFormListPaneModel::addExtraColumn(QString header, extra_column_handler&& handler) {
   size_t at = ColumnCount + this->extra_columns.list.size();
   this->beginInsertColumns({}, at, at);

   this->extra_columns.list.push_back({
      .handler     = handler,
      .header_text = header,
   });

   // Update the "any getters take a loaded form" flag:
   if (auto& flag = this->extra_columns.any_getters_take_loaded_form; !flag) {
      if (std::holds_alternative<extra_column_handler_by_data>(handler)) {
         flag = true;
      }
   }

   auto& col = this->extra_columns.list.back();
   for (auto* item : this->children) {
      this->_recacheNewlyAppendedExtraColumn(*item, col.handler);
   }

   this->endInsertColumns();
}
void DKFormListPaneModel::removeExtraColumn(size_t which) {
   if (which >= this->extra_columns.list.size())
      return;
   const size_t at = ColumnCount + which;
   this->beginRemoveColumns({}, at, at);

   // Update the "any getters take a loaded form" flag:
   if (auto& flag = this->extra_columns.any_getters_take_loaded_form; flag) {
      auto& col = this->extra_columns.list[which];
      if (std::holds_alternative<extra_column_handler_by_data>(col.handler)) {
         bool any = false;
         for (size_t i = 0; i < this->extra_columns.list.size(); ++i) {
            if (i == which)
               continue;
            auto& col = this->extra_columns.list[i];
            if (std::holds_alternative<extra_column_handler_by_data>(col.handler)) {
               any = true;
               break;
            }
         }
         flag = any;
      }
   }
   
   const size_t cached_index_to_delete = Item::cache_index_for_extra_column(which);
   this->extra_columns.list.removeAt(which);
   for (auto* item : this->children) {
      item->columnText.erase(item->columnText.begin() + cached_index_to_delete);
   }
   
   this->endRemoveColumns();
}
#pragma endregion