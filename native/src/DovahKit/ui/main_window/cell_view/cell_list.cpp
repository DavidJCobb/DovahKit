#include "cell_list.h"
#include <QHeaderView>
#include <QMimeData>
#include "editor/core.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/helpers/form_stub_drag_drop.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/form_stub.h"
#include "widgets/DKHeaderView.h"

#include "editor/subsystems/worldedit/core.h"

CellListModelItem::CellListModelItem(const dovah::form_stub* stub) {
   this->stub = stub;
   this->update();
}
void CellListModelItem::update() {
   auto stub = this->stub;
   this->editorID = QString::fromUtf8(stub->get_editor_id());
   this->formID   = stub->formID;
   stub->get_grid_coordinates(this->gridX, this->gridY);
   //
   this->is_active   = stub->is_edited_or_in_active_file() && !stub->test_record_flags(dovah::tes_file_record_header::flag::partial);
   this->is_injected = stub->is_injected();
   //
   this->updateRenderWindowState();
}
void CellListModelItem::updateRenderWindowState() {
   auto& worldedit = dovahkit::subsystems::worldedit::core::get();
   this->render_window_loaded = worldedit.is_cell_loaded(stub);
   if (this->render_window_loaded) {
      this->render_window_current = worldedit.is_current_cell(stub);
   } else {
      this->render_window_current = false;
   }
}

#pragma region CellListModel
CellListModel::CellListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &CellListModel::clear);
   QObject::connect(&editor, &DovahKitCore::formCreated,          this, &CellListModel::formCreated);
   QObject::connect(&editor, &DovahKitCore::formModified,         this, &CellListModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &CellListModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,       this, &CellListModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, [this]() { this->rebuild(this->worldspace); });
   //
   auto& worldedit = dovahkit::subsystems::worldedit::core::get_or_create();
   QObject::connect(&worldedit, &dovahkit::subsystems::worldedit::core::cellLoaded, this, [this](dovah::form_stub& cell) {
      this->cellRenderWindowLoadedStateChanged(cell, true);
   });
   QObject::connect(&worldedit, &dovahkit::subsystems::worldedit::core::cellUnloaded, this, [this](dovah::form_stub& cell) {
      this->cellRenderWindowLoadedStateChanged(cell, false);
   });
   QObject::connect(&worldedit, &dovahkit::subsystems::worldedit::core::currentCellChanged,      this, &CellListModel::renderWindowCurrentCellChanged);
   QObject::connect(&worldedit, &dovahkit::subsystems::worldedit::core::crossedIntoExteriorCell, this, &CellListModel::renderWindowCurrentCellChanged);
}

void CellListModel::cellRenderWindowLoadedStateChanged(const dovah::form_stub& cell, bool loaded) {
   if (this->worldspace != cell.get_parent_form())
      return;
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == &cell) {
         item->render_window_loaded = loaded;
         if (!loaded)
            item->render_window_current = false;
         this->emitRowChanged(i);
         break;
      }
   }
}
void CellListModel::formCreated(const dovah::form_stub* stub) {
   if (stub->form_type != dovah::form_type::cell)
      return;
   auto* parent = stub->get_parent_form();
   if (this->worldspace) {
      if (parent != this->worldspace)
         return;
   } else {
      if (parent)
         return;
   }
   this->insertItem(stub, false);
}
void CellListModel::formModified(const dovah::form_stub* stub) {
   if (stub->form_type != dovah::form_type::cell)
      return;
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub == stub) {
         item->update();
         this->emitRowChanged(i);
         break;
      }
   }
}
void CellListModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   if (stub->form_type != dovah::form_type::cell)
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
void CellListModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
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
void CellListModel::renderWindowCurrentCellChanged(const dovah::form_stub* current) {
   bool updated_old = false;
   bool updated_new = false;
   //
   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      //
      if (item->stub == current) {
         if (item->render_window_current)
            break;
         item->render_window_current = true;
         this->emitRowChanged(i);
         updated_new = true;
      } else if (item->render_window_current) {
         item->render_window_current = false;
         this->emitRowChanged(i);
         updated_old = true;
      }
      if (updated_new && updated_old)
         break;
   }
}

QModelIndex CellListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex CellListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int CellListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int CellListModel::columnCount(const QModelIndex& item) const {
   return 4;
}
Qt::ItemFlags CellListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsDragEnabled;
}
QVariant CellListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item    = (item_type*)index.internalPointer();
   auto column  = index.column();
   bool edited  = item->is_active;
   bool deleted = item->stub->is_deleted();
   if (role == Qt::FontRole) {
      auto font = QFont();
      if (column == ColumnName) {
         if (item->editorID.isEmpty()) {
            font.setItalic(true);
         }
      }
      if (item->render_window_current) {
         font.setBold(true);
      }
      return font;
   }
   if (role == RenderWindowRole) {
      if (item->render_window_current)
         return -10;
      if (item->render_window_loaded)
         return -5;
      return 0;
   }
   if (role == SortOverrideRole) {
      int magnitude = 0;
      if (column == ColumnName) {
         if (item->editorID.isEmpty())
            magnitude += 1;
      }
      return magnitude;
   }
   switch (column) {
      case ColumnName: // editor ID
         switch (role) {
            case Qt::ToolTipRole:
            case Qt::DisplayRole:
            case SortingRole:
            case FilteringRole:
               {
                  QString text = item->editorID;
                  if (text.isEmpty())
                     text = tr("Unnamed Cell", "cell view cell list");
                  if (deleted && role == Qt::DisplayRole)
                     text += tr(" * ", "edited form ID marker");
                  return text;
               }
         }
         break;
      case ColumnFormID: // form ID
         switch (role) {
            case Qt::DisplayRole:
               return editor_helpers::form_id_to_string(item->formID) + ((edited || deleted) ? tr(" * ", "edited form ID marker") : "") + (deleted ? tr("D", "deleted form ID marker") : "");
            case Qt::ForegroundRole:
               if (item->is_injected)
                  return QColor::fromRgb(0x309000);
               break;
            case FilteringRole:
               return editor_helpers::form_id_to_string(item->formID);
            case SortingRole:
               return item->formID;
         }
         break;
      case ColumnGridX: // grid X
         switch (role) {
            case Qt::DisplayRole:
            case SortingRole:
               return item->gridX;
            case FilteringRole:
               return QVariant(); // don't allow filtering by the grid coordinates
         }
         break;
      case ColumnGridY: // grid Y
         switch (role) {
            case Qt::DisplayRole:
            case SortingRole:
               return item->gridY;
            case FilteringRole:
               return QVariant(); // don't allow filtering by the grid coordinates
         }
         break;
   }
   return QVariant();
}
QMimeData* CellListModel::mimeData(const QModelIndexList& indexes) const {
   QMimeData* out = new QMimeData;

   std::vector<dovah::form_stub*> stubs;

   auto& list = this->children;
   auto  size = list.size();
   for (const QModelIndex& index : indexes) {
      if (index.column() != 0) // row selection + table with multiple columns = multiple indices that represent the same row, one for each column. skip the extras
         continue;
      if (index.isValid()) {
         auto i = index.row();
         if (i < 0 || i >= size)
            continue;
         auto* item = this->children[index.row()];
         if (!item->stub)
            continue;

         stubs.push_back(const_cast<dovah::form_stub*>(item->stub));
      }
   }

   editor_helpers::add_form_stubs_to_mime_data(*out, stubs);

   return out;
}
QStringList CellListModel::mimeTypes() const {
   return QStringList({
      QString(editor_helpers::form_stub_array_mime_type),
      QString(editor_helpers::single_form_stub_mime_type)
   });
}
//
QVariant CellListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case ColumnName:   return tr("Editor ID", "cell view cell list");
            case ColumnFormID: return tr("Form ID",   "cell view cell list");
            case ColumnGridX:  return tr("X", "cell view cell list");
            case ColumnGridY:  return tr("Y", "cell view cell list");
         }
         break;
   }
   return QVariant();
}

void CellListModel::emitRowChanged(int i) {
   auto root  = QModelIndex();
   auto start = this->index(i, 0, root);
   auto end   = this->index(i, this->columnCount(root) - 1, root);
   emit dataChanged(start, end);
}
void CellListModel::insertItem(const dovah::form_stub* stub, bool queued) {
   if (!stub)
      return;
   if (stub->formID == dovah::hardcoded_form_ids::NavmeshGenCell)
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

void CellListModel::clear() {
   this->beginResetModel();
   this->worldspace = nullptr;
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->endResetModel();
}
void CellListModel::rebuild(const dovah::form_stub* worldspace) {
   this->clear();
   //
   auto& editor = DovahKitCore::get();
   if (!editor.has_data())
      return;
   //
   this->worldspace = worldspace;
   if (worldspace) {
      editor.for_each_form_of_type(dovah::form_type::cell, [worldspace, this](dovah::form_stub* stub) {
         if (stub->get_parent_form() != worldspace)
            return false;
         this->insertItem(stub, true);
         return false;
      });
   } else {
      editor.for_each_form_of_type(dovah::form_type::cell, [this](dovah::form_stub* stub) {
         if (stub->get_parent_form())
            return false;
         this->insertItem(stub, true);
         return false;
      });
   }
   auto& queue = this->queued_additions;
   auto  count = queue.size();
   if (!count)
      return;
   auto& list  = this->children;
   auto  first = list.size();
   auto  last  = first + (count - 1);
   this->beginInsertRows(QModelIndex(), first, last);
   for (auto* item : queue)
      list.push_back(item);
   queue.clear();
   this->endInsertRows();
}
#pragma endregion

#pragma region CellListModelProxy
CellListModelProxy::CellListModelProxy(QObject* parent) : QSortFilterProxyModel(parent) {
   this->setFilterCaseSensitivity(Qt::CaseInsensitive);
   this->setFilterRole(CellListModel::FilteringRole);
   this->setFilterKeyColumn(-1);
   this->setSortCaseSensitivity(Qt::CaseInsensitive);
   this->setSortRole(CellListModel::SortingRole);
}
bool CellListModelProxy::lessThan(const QModelIndex& left, const QModelIndex& right) const {
   auto source    = this->sourceModel();
   auto sort_role = this->sortRole();
   //
   int over_a = 0;
   int over_b = 0;
   if (this->_loadedCellsAtTop) {
      over_a = source->data(left,  CellListModel::RenderWindowRole).toInt();
      over_b = source->data(right, CellListModel::RenderWindowRole).toInt();
   }
   over_a += source->data(left,  CellListModel::SortOverrideRole).toInt();
   over_b += source->data(right, CellListModel::SortOverrideRole).toInt();
   if (over_a != over_b) {
      return over_a < over_b;
   }
   //
   QVariant lhs = source->data(left,  sort_role);
   QVariant rhs = source->data(right, sort_role);
   if (lhs.typeId() != QMetaType::QString && lhs.canConvert<int>() && rhs.typeId() == lhs.typeId()) {
      return lhs.toInt() < rhs.toInt();
   }
   return QString::localeAwareCompare(lhs.toString(), rhs.toString()) < 0;
}
void CellListModelProxy::setLoadedCellsAtTop(bool v) {
   if (v == this->_loadedCellsAtTop)
      return;
   this->_loadedCellsAtTop = v;
   this->invalidate();
}
#pragma endregion

#pragma region CellList
CellList::CellList(QWidget* parent) : QTableView(parent) {
   auto underlying = new model_type;
   auto proxy      = new CellListModelProxy(this);
   proxy->setSourceModel(underlying);
   proxy->setDynamicSortFilter(true);
   this->setModel(proxy);
   this->verticalHeader()->setDefaultSectionSize(0);
   this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
   this->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
   {
      auto* header = new DKHeaderView(Qt::Horizontal, this);
      header->setFlexResizeEnabled(true);
      {
         auto* old = this->horizontalHeader();
         header->setHighlightSections(old->highlightSections());
         header->setSortIndicatorShown(old->isSortIndicatorShown());
         header->setSectionsClickable(old->sectionsClickable());
      }
      this->setHorizontalHeader(header);
      //
      auto metrics = QFontMetrics(this->font());
      header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
      header->setMinimumSectionSize(2);
      header->setColumnFlex(CellListModel::ColumnName,   1, 0);
      header->setColumnFlex(CellListModel::ColumnFormID, 0, 0, metrics.boundingRect("00000000").width() * 1.5F + 4);
      header->setColumnFlex(CellListModel::ColumnGridX,  0, 0, metrics.boundingRect("00").width() * 1.5F + 4);
      header->setColumnFlex(CellListModel::ColumnGridY,  0, 0, metrics.boundingRect("00").width() * 1.5F + 4);
      header->modSectionSizeTo(CellListModel::ColumnFormID, 4); // mimics a user resize and shrinks the column
      for(int i = 0; i <= CellListModel::ColumnGridY; ++i)
         header->setSectionResizeMode(i, QHeaderView::Interactive);
      header->setStretchLastSection(false);
   }
   this->sortByColumn(CellListModel::ColumnName, Qt::AscendingOrder);
   
   QObject::connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
      if (auto* stub = this->formStub())
         emit this->currentCellChanged(stub);
   });
   QObject::connect(this, &QAbstractItemView::doubleClicked, [this](const QModelIndex& index) {
      auto proxy = (proxy_type*)this->model();
      auto qmi   = proxy->mapToSource(index);
      if (!qmi.isValid())
         return;
      auto* item = (model_item_type*)qmi.internalPointer();
      if (!item)
         return;
      if (item->stub)
         emit this->renderRequested(const_cast<dovah::form_stub*>(item->stub)); // const-cast: stub should not be modified by anything inside this system; don't care about what outside code does
   });
};
void CellList::setWorldspacePicker(const FormsOfTypeCombobox* picker) {
   if (picker == this->_worldspaceSelector)
      return;
   if (this->_worldspaceSelector)
      QObject::disconnect(this->_worldspaceSelector, nullptr, this, nullptr);
   this->_worldspaceSelector = picker;
   if (picker) {
      QObject::connect(picker, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CellList::rebuildModel);
   }
}
dovah::bare_form_id_t CellList::formID() const noexcept {
   const auto* item = this->_getCurrentItem();
   if (!item || !item->stub)
      return 0;
   return item->stub->formID;
}
dovah::form_stub* CellList::formStub() const noexcept {
   const auto* item = this->_getCurrentItem();
   if (!item || !item->stub)
      return nullptr;
   return const_cast<dovah::form_stub*>(item->stub); // const-cast: stub should not be modified by anything inside this system; don't care about what outside code does
}

void CellList::setLoadedCellsAtTop(bool v) {
   auto proxy = (proxy_type*)this->model();
   proxy->setLoadedCellsAtTop(v);
}

void CellList::selectCell(dovah::form_stub* stub) {
   if (stub && stub->form_type != dovah::form_type::cell)
      return;
   auto* proxy     = (proxy_type*)this->model();
   auto* sel_model = this->selectionModel();
   if (!stub) {
      sel_model->select(QModelIndex{}, QItemSelectionModel::SelectionFlag::Clear);
      return;
   }
   size_t count = proxy->rowCount({});
   for (size_t i = 0; i < count; ++i) {
      const auto  proxy_qmi  = proxy->index(i, 0, {});
      const auto  source_qmi = proxy->mapToSource(proxy_qmi);
      const auto* item       = (model_item_type*)source_qmi.internalPointer();
      if (!item)
         continue;
      if (item->stub == stub) {
         sel_model->select(
            QItemSelection{
               proxy_qmi,
               proxy_qmi.siblingAtColumn(proxy->columnCount({}) - 1)
            },
            QItemSelectionModel::SelectionFlag::ClearAndSelect
         );
         return;
      }
   }
}

void CellList::rebuildModel() {
   auto m = this->unwrappedModel();
   if (!m)
      return;
   const dovah::form_stub* stub = nullptr;
   if (this->_worldspaceSelector) {
      auto formID = this->_worldspaceSelector->formID();
      if (formID) {
         auto& editor = DovahKitCore::get();
         stub = editor.get_form(formID);
         if (stub->form_type != dovah::form_type::worldspace)
            stub = nullptr;
      }
   }
   m->rebuild(stub);
   //
   auto header = this->horizontalHeader();
   header->setSectionHidden(2, !stub); // don't show grid coordinates for interior cells, as they do not have any
   header->setSectionHidden(3, !stub); // 
}
void CellList::clear() {
   auto m = this->unwrappedModel();
   if (!m)
      return;
   m->clear();
}
CellList::model_item_type* CellList::_getCurrentItem() const noexcept {
   auto proxy  = (proxy_type*)this->model();
   auto select = this->selectionModel()->selection().indexes();
   if (select.size() <= 0)
      return nullptr;
   auto idx = proxy->mapToSource(select[0]);
   return (model_item_type*)idx.internalPointer();
}
#pragma endregion