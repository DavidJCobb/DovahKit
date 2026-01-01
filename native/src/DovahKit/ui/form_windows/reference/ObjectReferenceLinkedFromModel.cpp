#include "./ObjectReferenceLinkedFromModel.h"
#include "helpers/vectors/re_sort_item_within.h"
#include "dovah/forms/components/extra_data/types/l/linked_ref.h"
#include "dovah/forms/ObjectReference.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "dovah/form_stubs/helpers/is_used_by.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"

ObjectReferenceLinkedFromModel::ObjectReferenceLinkedFromModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ObjectReferenceLinkedFromModel::_on_data_abandoned);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formRenumbered, this, [this](dovah::form_stub* stub) { this->_on_form_renumbered(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &ObjectReferenceLinkedFromModel::_on_all_forms_renumbered);
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex ObjectReferenceLinkedFromModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex ObjectReferenceLinkedFromModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex ObjectReferenceLinkedFromModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int ObjectReferenceLinkedFromModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int ObjectReferenceLinkedFromModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant ObjectReferenceLinkedFromModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_data.size())
            return {};
         auto& src = this->_data[index.row()];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (index.column()) {
                  case Column::KeywordName:
                     return src.cached.keyword;
                  case Column::RefName:
                     return src.cached.ref;
                  case Column::RefFormID:
                     if (!src.ref)
                        break;
                     return editor_helpers::form_id_to_string(src.ref->formID);
               }
               break;
            case FormStubRole:
               switch (index.column()) {
                  case Column::KeywordName:
                     return QVariant::fromValue(src.keyword);
                  case Column::RefName:
                  case Column::RefFormID:
                     return QVariant::fromValue(src.ref);
               }
               break;
            case KeywordRole:
               return QVariant::fromValue(src.keyword);
            case RefRole:
               return QVariant::fromValue(src.ref);
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags ObjectReferenceLinkedFromModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant ObjectReferenceLinkedFromModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      switch (section) {
         case Column::KeywordName:
            return tr("Keyword");
         case Column::RefName:
            return tr("ObjectReference");
         case Column::RefFormID:
            return tr("Ref ID");
      }
      return {};
   }
#pragma endregion

void ObjectReferenceLinkedFromModel::setSubject(dovah::form_stub* ref) {
   this->beginResetModel();
   this->_data.clear();
   this->_subject = ref;
   if (!ref) {
      this->endResetModel();
      return;
   }
   for (const auto& [_, use] : ref->inbound) {
      auto* user = use.other;
      assert(!!user);
      if (user == ref) // illegal for a ref to link to itself, so don't even bother checking self-uses
         continue;
      if (!dovah::form_type_is_reference(user->form_type))
         continue;
      auto loaded = user->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
      if (!loaded)
         continue;
      auto* extra = loaded->extra_data.get<extra_data_type>();
      if (!extra)
         continue;
      for (auto& link : extra->links) {
         if (!link.ref)
            continue;
         if (link.ref.get_form_stub() != this->_subject)
            continue;
         Row row;
         row.keyword = link.keyword.get_form_stub();
         row.ref     = user;
         if (row.keyword)
            row.cached.keyword = _name_of(*row.keyword);
         row.cached.ref = _name_of(*user);
         this->_data.push_back(std::move(row));
      }
   }
   std::sort(
      this->_data.begin(),
      this->_data.end(),
      _sort_comparator
   );
   this->endResetModel();
}

void ObjectReferenceLinkedFromModel::_on_data_abandoned() {
   this->beginResetModel();
   this->_data.clear();
   this->_subject = nullptr;
   this->endResetModel();
}
void ObjectReferenceLinkedFromModel::_on_form_deleted(dovah::form_stub& stub) {
   if (this->_subject == nullptr) {
      assert(this->_data.empty());
      return;
   }
   if (this->_subject == &stub) {
      this->setSubject(nullptr);
      return;
   }
   if (stub.form_type == dovah::form_type::keyword) {
      for (size_t i = 0; i < this->_data.size(); ++i) {
         if (this->_data[i].keyword == &stub) {
            this->beginRemoveRows({}, i, i);
            this->_data.erase(this->_data.begin() + i);
            this->endRemoveRows();
            break;
         }
      }
      return;
   }
   if (dovah::form_type_is_reference(stub.form_type)) {
      for (size_t i = 0; i < this->_data.size(); ++i) {
         if (this->_data[i].ref == &stub) {
            this->beginRemoveRows({}, i, i);
            this->_data.erase(this->_data.begin() + i);
            this->endRemoveRows();
         }
      }
      return;
   }
}
void ObjectReferenceLinkedFromModel::_on_form_modified(dovah::form_stub& stub) {
   if (this->_subject == nullptr) {
      assert(this->_data.empty());
      return;
   }
   if (stub.form_type == dovah::form_type::keyword) {
      this->_on_keyword_modified(stub);
      return;
   }
   if (dovah::form_type_is_reference(stub.form_type)) {
      if (!dovah::form_stub_helpers::is_used_by(*this->_subject, stub)) {
         _remove_all_links_from(stub);
         return;
      }
      auto loaded = stub.load().ptr_cast<dovah::loaded_forms::ObjectReference>();
      if (!loaded) {
         _remove_all_links_from(stub);
         return;
      }
      const auto* extra = loaded->extra_data.get<extra_data_type>();
      if (!extra) {
         _remove_all_links_from(stub);
         return;
      }

      QString cached_editor_id = _name_of(stub);

      // Key is a KYWD form. Value is a bool indicating whether to insert (true) or 
      // remove (false) a link to our subject ref.
      std::unordered_map<dovah::form_stub*, bool> changes;

      for (size_t i = 0; i < this->_data.size(); ++i) {
         auto& dst_item = this->_data[i];
         if (dst_item.ref != &stub)
            continue;
         dst_item.cached.ref = cached_editor_id;
         auto qmi = this->index(i, Column::RefName, {});
         emit dataChanged(qmi, qmi);
         //
         // Detect pairs that need to be removed.
         //
         bool found = false;
         for (auto& src_item : extra->links) {
            if (src_item.ref.get_form_stub() != this->_subject)
               continue;
            if (src_item.keyword.get_form_stub() != dst_item.keyword)
               continue;
            found = true;
            break;
         }
         if (!found)
            changes[dst_item.keyword] = false;
      }
      //
      // Detect pairs that need to be added.
      //
      for (auto& src_item : extra->links) {
         if (src_item.ref.get_form_stub() != this->_subject)
            continue;
         bool found = false;
         for (auto& dst_item : this->_data) {
            if (dst_item.ref != &stub)
               continue;
            if (dst_item.keyword != src_item.keyword.get_form_stub())
               continue;
            found = true;
            break;
         }
         if (!found) {
            changes[src_item.keyword.get_form_stub()] = true;
         }
      }
      //
      // Carry out removals and insertions.
      //
      for (auto [keyword_stub, insert] : changes) {
         if (insert) {
            Row added;
            added.keyword = keyword_stub;
            added.ref     = &stub;
            if (keyword_stub)
               added.cached.keyword = _name_of(*keyword_stub);
            added.cached.ref = cached_editor_id;

            auto it = _insertion_point_for(added);
            auto i  = std::distance(this->_data.begin(), it);
            this->beginInsertRows({}, i, i);
            this->_data.insert(it, std::move(added));
            this->endInsertRows();
         } else {
            for (size_t i = 0; i < this->_data.size(); ++i) {
               auto& dst_item = this->_data[i];
               if (dst_item.keyword != keyword_stub)
                  continue;
               if (dst_item.ref != &stub)
                  continue;
               this->beginRemoveRows({}, i, i);
               this->_data.erase(this->_data.begin() + i);
               this->endRemoveRows();
               break;
            }
         }
      }
   }
}
void ObjectReferenceLinkedFromModel::_on_form_renumbered(dovah::form_stub& stub) {
   if (this->_subject == nullptr) {
      assert(this->_data.empty());
      return;
   }
   if (!dovah::form_type_is_reference(stub.form_type))
      return;
   for (size_t i = 0; i < this->_data.size(); ++i) {
      auto& pair = this->_data[i];
      if (pair.ref == &stub) {
         auto qmi = this->index(i, Column::RefFormID, {});
         emit dataChanged(qmi, qmi);
         break;
      }
   }
}
void ObjectReferenceLinkedFromModel::_on_all_forms_renumbered() {
   if (this->_subject == nullptr) {
      assert(this->_data.empty());
      return;
   }
   auto size = this->_data.size();

   {
      auto tl = this->index(0, Column::RefFormID, {});
      auto br = this->index(size - 1, Column::RefFormID, {});
      emit dataChanged(tl, br);
   }
}

void ObjectReferenceLinkedFromModel::_on_keyword_modified(dovah::form_stub& stub) {
   QString keyword_editor_id;
   //
   // We sort the table by keyword, but multiple rows may use the same keyword. 
   // We re-sort each row as it's found, which means that after the sort, we 
   // no longer know which rows we have or haven't checked. Therefore, we need 
   // a doubly-nested loop: every time we find a keyword that needs updating, 
   // re-crawl the entire list.
   //
   bool moves_pending = true;
   while (moves_pending) {
      moves_pending = false;
      for (size_t i = 0; i < this->_data.size(); ++i) {
         auto& item = this->_data[i];
         if (item.keyword == &stub && item.cached.keyword != keyword_editor_id) { // QString is interned so this string comparison is fast
            item.cached.keyword = keyword_editor_id;
            auto qmi = this->index(i, Column::KeywordName, {});
            emit dataChanged(qmi, qmi);
            this->_re_sort_item(i);
            //
            // Re-sorting this item means we no longer know which items 
            // we've checked and which ones we haven't. We have to re-run 
            // the loop on the entire list.
            //
            moves_pending = true;
            break;
         }
      }
   }
}
void ObjectReferenceLinkedFromModel::_remove_all_links_from(dovah::form_stub& stub) {
   auto&  list = this->_data;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto& item = list[i];
      if (item.ref == &stub) {
         this->beginRemoveRows({}, i, i);
         list.erase(list.begin() + i);
         --size;
         --i;
         this->endRemoveRows();
      }
   }
}

decltype(ObjectReferenceLinkedFromModel::_data)::iterator ObjectReferenceLinkedFromModel::_insertion_point_for(const Row& item) {
   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      item,
      _sort_comparator
   );
}
void ObjectReferenceLinkedFromModel::_re_sort_item(size_t from) {
   auto& list = this->_data;
   if (from >= list.size())
      return;
   bool moved = false;
   cobb::vectors::re_sort_item_within(
      list,
      list.begin() + from,
      _sort_comparator,
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
/*static*/ bool ObjectReferenceLinkedFromModel::_sort_comparator(const Row& a, const Row& b) {
   if (!a.keyword)
      return b.keyword != nullptr;
   if (!b.keyword)
      return false;
   if (a.keyword == b.keyword) {
      return a.cached.ref.localeAwareCompare(b.cached.ref) < 0;
   }
   return a.cached.keyword.localeAwareCompare(b.cached.keyword) < 0;
}

/*static*/ QString ObjectReferenceLinkedFromModel::_name_of(const dovah::form_stub& stub) {
   if (dovah::form_type_is_reference(stub.form_type)) {
      if (stub.editorID.empty()) {
         if (auto* base = dovah::form_stub_helpers::get_base_form(&stub))
            return QString::fromStdString(base->editorID);
      }
   }
   return QString::fromStdString(stub.editorID);
}