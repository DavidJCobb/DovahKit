#include "./form_table_source_model.h"
#include "./form_model_item.h"
#include <QColor>
#include <QFont>
#include <QMimeData>
#include "dovah/form_stub.h"
#include "editor/core.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/helpers/form_stub_drag_drop.h"
#include "editor/subsystems/form_info_cache/core.h"

namespace ui::object_window {
   form_table_source_model::form_table_source_model(QObject* parent) : QAbstractTableModel(parent) {
      // Ensure that the FIC exists, and hooks `formModified`, before we do. That way, when we hook 
      // `formModified`, any filters that depend on the FIC will be checking up-to-date data.
      auto& fic = dovahkit::subsystems::form_info_cache::core::get_or_create();

      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,      this, &form_table_source_model::clear);
      QObject::connect(&editor, &DovahKitCore::formCreated,              this, &form_table_source_model::formCreated);
      QObject::connect(&editor, &DovahKitCore::formModificationImminent, this, &form_table_source_model::formModificationImminent);
      QObject::connect(&editor, &DovahKitCore::formModified,             this, &form_table_source_model::formModified);
      QObject::connect(&editor, &DovahKitCore::formDeletionImminent,     this, &form_table_source_model::formDeletionImminent);
      QObject::connect(&editor, &DovahKitCore::formRenumbered,           this, &form_table_source_model::formRenumbered);
   }

   void form_table_source_model::formCreated(dovah::form_stub* stub) {
      if (!this->form_types.contains(stub->form_type))
         return;
      auto* item  = new item_type(stub);
      auto& list  = this->children;
      auto  first = list.size();
      this->beginInsertRows({}, first, first);
      list.push_back(item);
      this->endInsertRows();
   }
   void form_table_source_model::formModificationImminent(const dovah::form_stub* user) {
      //
      // The (user) form is about to be changed, and those changes may result in it no longer 
      // using some other form. We need to take note of all of the forms that it currently 
      // uses, so that we can update them after the (user) form is changed.
      //
      auto& list = this->forms_pending_use_info_update;
      for (auto& pair : user->outbound) {
         auto stub = pair.second.other;
         if (!stub)
            continue;
         if (!list.contains(stub))
            list.push_back(stub);
      }
   }
   void form_table_source_model::formModified(const dovah::form_stub* stub) {
      QVector<dovah::form_stub*> used;
      for (auto& pair : stub->outbound) {
         auto* other = pair.second.other;
         if (other)
            used.push_back(other);
      }
      //
      auto  parent_index = QModelIndex();
      auto& list = this->children;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto* item = list[i];
         if (item->stub == stub) {
            item->update();
            auto root  = QModelIndex();
            auto start = this->index(i, 0, root);
            auto end   = this->index(i, this->columnCount(root) - 1, root);
            emit dataChanged(start, end);
            break;
         } else if (used.contains(item->stub)) {
            //
            // Update any other forms that need their use counts used because (stub) was 
            // changed to use them.
            //
            if (item->update_user_count()) {
               auto root  = QModelIndex();
               auto start = this->index(i, 0, root);
               auto end   = this->index(i, this->columnCount(root) - 1, root);
               emit dataChanged(start, end);
            }
         }
      }
      //
      this->doUseInfoUpdate();

      this->_emit_data_changed_on(*stub);
   }
   void form_table_source_model::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
      auto& list = this->children;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto* item = list[i];
         if (item->stub == stub) {
            if (is_just_flagged) {
               auto index = this->index(i, 0, QModelIndex());
               emit dataChanged(index, index); // force a redraw, which will show the "deleted" flag
               break;
            }
            this->beginRemoveRows(QModelIndex(), i, i);
            list.remove(i);
            this->endRemoveRows();
            break;
         }
      }
   }
   void form_table_source_model::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
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
            return;
         }
      }
   }

   void form_table_source_model::_emit_data_changed_on(const dovah::form_stub& stub) {
      //
      // We could be filtering forms by some characteristic that just changed 
      // for this form. Poke the proxy model to re-check it.
      //
      auto&  list = this->children;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item->stub == &stub) {
            auto root  = QModelIndex{};
            auto start = this->index(i, 0, root);
            auto end   = this->index(i, this->columnCount(root) - 1, root);
            emit dataChanged(start, end);
            break;
         }
      }
   }

   QModelIndex form_table_source_model::index(dovah::form_stub* stub) const {
      int size = this->children.size();
      for (int i = 0; i < size; ++i)
         if (this->children[i]->stub == stub)
            return this->index(i, 0, {});
      return {};
   }

   #pragma region QAbstractItemModel overrides
      #pragma region Hierarchy
         /*virtual*/ QModelIndex form_table_source_model::index(int row, int column, const QModelIndex& parent) const /*override*/ {
            if (!this->hasIndex(row, column, parent))
               return {};
            item_type* item = this->children.value(row);
            if (item)
               return this->createIndex(row, column, item);
            return {};
         }
         /*virtual*/ QModelIndex form_table_source_model::parent(const QModelIndex& index) const /*override*/ {
            return {};
         }
         /*virtual*/ int form_table_source_model::rowCount(const QModelIndex& parent) const /*override*/ {
            if (parent.isValid())
               return 0;
            return this->children.size();
         }
         /*virtual*/ int form_table_source_model::columnCount(const QModelIndex& item) const /*override*/ {
            return ColumnCount;
         }
      #pragma endregion
      #pragma region Item data
         /*virtual*/ Qt::ItemFlags form_table_source_model::flags(const QModelIndex& index) const /*override*/ {
            if (!index.isValid())
               return Qt::NoItemFlags;
            return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsDragEnabled | Qt::ItemFlag::ItemNeverHasChildren;
         }
         /*virtual*/ QVariant form_table_source_model::data(const QModelIndex& index, int role) const /*override*/ {
            if (!index.isValid())
               return {};
            auto item = data_for_qmi(index);
            if (!item->stub)
               return {};
            auto column  = index.column();
            bool edited  = item->is_active;
            bool deleted = item->stub->is_deleted();
            bool none    = item->is_none;
            switch (role) {
               case Qt::DisplayRole:
                  switch (column) {
                     case Column::EditorID:
                        if (none) {
                           return tr("<non-existent form>%1", "object window listing for none stubs")
                              .arg((edited || deleted) ? tr(" * ", "edited form editor ID marker") : "");
                        }
                        return tr("%1%2")
                           .arg(item->editor_id)
                           .arg((edited || deleted) ? tr(" * ", "edited form editor ID marker") : "");
                     case Column::FormID:
                        return editor_helpers::form_id_to_string(item->form_id) + ((edited || deleted) ? tr(" * ", "edited form ID marker") : "") + (deleted ? tr("D", "deleted form ID marker") : "");
                        case Column::UserCount:
                        return item->user_count;
                  }
                  break;
               case Qt::DecorationRole:
                  if (column == Column::EditorID) {
                     //
                     // TODO: icons per form type
                     //
                  }
                  break;
               case Qt::FontRole:
                  if (column == Column::EditorID && none) { // show none-stubs in italics
                     QFont font;
                     font.setItalic(true);
                     return font;
                  }
                  break;
               case Qt::ForegroundRole:
                  if (column == Column::FormID && item->is_injected) // show injected forms' IDs in color
                     return QColor::fromRgb(0x309000);
                  break;
               case RawDataRole:
                  switch (column) {
                     case Column::EditorID:  return item->editor_id;
                     case Column::FormID:    return item->form_id;
                     case Column::UserCount: return item->user_count;
                  }
                  break;
               case FilterableTextRole: // used for filtering
                  switch (column) {
                     case Column::EditorID:
                        if (none)
                           return {};
                        return item->editor_id;
                     case Column::FormID:
                        return editor_helpers::form_id_to_string(item->form_id);
                     case Column::UserCount:
                        return {}; // don't allow filtering by the use count
                  }
                  break;
            }
            return {};
         }
      #pragma endregion
      /*virtual*/ QVariant form_table_source_model::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         switch (role) {
            case Qt::DisplayRole:
               switch (section) {
                  case Column::EditorID:  return tr("Editor ID", "column header");
                  case Column::FormID:    return tr("Form ID",   "column header");
                  case Column::UserCount: return tr("Users",     "column header");
               }
               break;
         }
         return {};
      }
      #pragma region Drag and drop
         /*virtual*/ QMimeData* form_table_source_model::mimeData(const QModelIndexList& indexes) const /*override*/ {
            //
            // We want the user to be able to drag selected forms out of the Object Window and into 
            // things like a FormList's form list. To do that, we have to send the data along with 
            // a MIME type. We've chosen "application/dovah-kit.form-id-array" as our MIME type; 
            // the underlying data is just form IDs binary-encoded, separated with null bytes.
            //

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

                  stubs.push_back(item->stub);
               }
            }

            editor_helpers::add_form_stubs_to_mime_data(*out, stubs);

            return out;
         }
         /*virtual*/ QStringList form_table_source_model::mimeTypes() const /*override*/ {
            return QStringList({
               QString(editor_helpers::form_stub_array_mime_type),
               QString(editor_helpers::single_form_stub_mime_type)
            });
         }
      #pragma endregion
   #pragma endregion

   void form_table_source_model::doUseInfoUpdate() {
      QModelIndex parent_index;
      auto& list = this->children;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto* item = list[i];
         auto* stub = item->stub;
         if (this->forms_pending_use_info_update.contains(stub)) {
            if (item->update_user_count()) {
               auto index = this->index(i, 0, parent_index);
               emit dataChanged(index, index);
            }
         }
      }
      this->forms_pending_use_info_update.clear();
   }

   void form_table_source_model::clear() {
      this->beginResetModel();
      for (auto* item : this->children)
         delete item;
      this->children.clear();
      this->forms_pending_use_info_update.clear();
      this->endResetModel();
   }
   void form_table_source_model::rebuild() {
      this->clear();

      if (this->form_types.empty())
         return;
      auto& editor = DovahKitCore::get();
      if (!editor.has_data())
         return;

      {
         bool any = false;
         for(auto ft : this->form_types)
            if (editor.count_forms_of_type(ft)) {
               any = true;
               break;
            }
         if (!any)
            return;
      }

      std::vector<item_type*> pending_additions;
      for (auto ft : this->form_types) {
         if (ft == dovah::form_type::none)
            editor.for_each_form_of_type(ft, [&pending_additions](dovah::form_stub* stub) {
               if (stub->is_none_stub()) {
                  pending_additions.push_back(new item_type(stub));
               }
               return false;
            });
         else
            editor.for_each_form_of_type(ft, [&pending_additions](dovah::form_stub* stub) {
               pending_additions.push_back(new item_type(stub));
               return false;
            });
      }
      //
      auto count = pending_additions.size();
      if (!count)
         return;
      auto first = this->children.size();
      auto last  = first + count - 1;
      //
      this->beginInsertRows({}, first, last);
      for (auto* item : pending_additions)
         this->children.push_back(item);
      this->endInsertRows();
   }
   void form_table_source_model::setBaseFormTypes(const form_type_set& types) {
      this->form_types = types;
   }

   const form_table_source_model::item_type* form_table_source_model::dataAtRow(int row) const noexcept {
      if (row < 0)
         return nullptr;
      if (row >= this->children.size())
         return nullptr;
      return this->children[row];
   }
}