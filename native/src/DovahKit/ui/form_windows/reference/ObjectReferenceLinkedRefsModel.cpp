#include "./ObjectReferenceLinkedRefsModel.h"
#include "helpers/vectors/re_sort_item_within.h"
#include "dovah/forms/components/extra_data/types/l/linked_ref.h"
#include "dovah/forms/ObjectReference.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"

ObjectReferenceLinkedRefsModel::ObjectReferenceLinkedRefsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ObjectReferenceLinkedRefsModel::_on_data_abandoned);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formRenumbered, this, [this](dovah::form_stub* stub) { this->_on_form_renumbered(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &ObjectReferenceLinkedRefsModel::_on_all_forms_renumbered);
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex ObjectReferenceLinkedRefsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex ObjectReferenceLinkedRefsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex ObjectReferenceLinkedRefsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int ObjectReferenceLinkedRefsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int ObjectReferenceLinkedRefsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant ObjectReferenceLinkedRefsModel::data(const QModelIndex& index, int role) const /*override*/ {
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
      /*virtual*/ Qt::ItemFlags ObjectReferenceLinkedRefsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant ObjectReferenceLinkedRefsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
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

void ObjectReferenceLinkedRefsModel::importData(const backend_form_type& form) {
   this->beginResetModel();
   this->_data.clear();
   if (auto* extra = form.extra_data.get<extra_data_type>()) {
      for (auto& link : extra->links) {
         if (!link.ref)
            continue;
         bool keyword_already_present = false;
         for (auto& item : this->_data) {
            if (item.keyword == link.keyword.get_form_stub()) {
               keyword_already_present = true;
               item.ref = link.ref.get_form_stub();
               break;
            }
         }
         if (!keyword_already_present) {
            auto& item = this->_data.emplace_back();
            item.keyword = link.keyword.get_form_stub();
            item.ref     = link.ref.get_form_stub();
            if (item.keyword) {
               item.cached.keyword = _name_of(*item.keyword);
            }
         }
      }
   }
   for (auto& item : this->_data) {
      assert(!!item.ref);
      item.cached.ref = _name_of(*item.ref);
   }
   std::sort(
      this->_data.begin(),
      this->_data.end(),
      [](const Mapping& a, const Mapping& b) {
         if (!a.keyword)
            return true;
         if (!b.keyword)
            return false;
         return a.cached.keyword.localeAwareCompare(b.cached.keyword) < 0;
      }
   );
   this->endResetModel();
}
void ObjectReferenceLinkedRefsModel::exportData(backend_form_type& form) const {
   if (this->_data.empty()) {
      form.extra_data.remove<extra_data_type>(form);
      return;
   }
   auto* extra = form.extra_data.get_or_create<extra_data_type>();
   extra->clear_contained_formIDs(form);
   for (auto& src : this->_data) {
      auto& dst = extra->links.emplace_back();
      dst.keyword.set(form, src.keyword);
      dst.ref.set(form, src.ref);
   }
}

QModelIndex ObjectReferenceLinkedRefsModel::index(const dovah::form_stub& keyword) const {
   if (keyword.form_type != dovah::form_type::keyword)
      return {};
   for (size_t i = 0; i < this->_data.size(); ++i)
      if (this->_data[i].keyword == &keyword)
         return this->index(i, 0, {});
   return {};
}
void ObjectReferenceLinkedRefsModel::setLink(dovah::form_stub* keyword, dovah::form_stub* ref) {
   for (size_t i = 0; i < this->_data.size(); ++i) {
      auto& item = this->_data[i];
      if (item.keyword == keyword) {
         if (!ref) {
            this->beginRemoveRows({}, i, i);
            this->_data.erase(this->_data.begin() + i);
            this->endRemoveRows();
            return;
         }
         if (item.ref == ref)
            return;
         item.ref = ref;
         auto qmi = this->index(i, Column::KeywordName, {});
         emit dataChanged(qmi, qmi);
         return;
      }
   }
   if (!ref)
      return;
   Mapping link;
   link.keyword = keyword;
   link.ref = ref;
   if (keyword)
      link.cached.keyword = _name_of(*keyword);
   link.cached.ref = _name_of(*ref);
   auto it = _insertion_point_for(link);
   auto i  = std::distance(this->_data.begin(), it);
   this->beginInsertRows({}, i, i);
   this->_data.insert(it, link);
   this->endInsertRows();
}
void ObjectReferenceLinkedRefsModel::setRow(size_t i, dovah::form_stub* keyword, dovah::form_stub* refr) {
   if (i >= this->_data.size())
      return;
   
   for (size_t j = 0; j < this->_data.size(); ++j) {
      if (j == i)
         continue;
      if (this->_data[j].keyword == keyword)
         return;
   }
   if (!refr) {
      this->beginRemoveRows({}, i, i);
      this->_data.erase(this->_data.begin() + i);
      this->endRemoveRows();
      return;
   }
   this->_data[i].keyword = keyword;
   this->_data[i].ref     = refr;
   emit dataChanged(this->index(i, 0, {}), this->index(i, ColumnCount - 1, {}));
}

std::vector<dovah::form_stub*> ObjectReferenceLinkedRefsModel::allKeywords() const {
   std::vector<dovah::form_stub*> out;
   out.reserve(this->_data.size());
   for (auto& item : this->_data)
      if (item.ref)
         out.push_back(item.keyword);
   return out;
}

void ObjectReferenceLinkedRefsModel::_on_data_abandoned() {
   this->beginResetModel();
   this->_data.clear();
   this->endResetModel();
}
void ObjectReferenceLinkedRefsModel::_on_form_deleted(dovah::form_stub& stub) {
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
void ObjectReferenceLinkedRefsModel::_on_form_modified(dovah::form_stub& stub) {
   if (stub.form_type == dovah::form_type::keyword) {
      for (size_t i = 0; i < this->_data.size(); ++i) {
         auto& item = this->_data[i];
         if (item.keyword != &stub)
            continue;
         item.cached.keyword = _name_of(stub);
         auto qmi = this->index(i, Column::KeywordName, {});
         emit dataChanged(qmi, qmi);
         this->_re_sort_item(i);
         break;
      }
      return;
   }
   if (dovah::form_type_is_reference(stub.form_type)) {
      QString cached;
      for (size_t i = 0; i < this->_data.size(); ++i) {
         auto& item = this->_data[i];
         if (item.ref != &stub)
            continue;
         if (cached.isEmpty())
            cached = _name_of(stub);
         item.cached.keyword = cached;
         auto qmi = this->index(i, Column::RefName, {});
         emit dataChanged(qmi, qmi);
      }
      return;
   }
}
void ObjectReferenceLinkedRefsModel::_on_form_renumbered(dovah::form_stub& stub) {
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
void ObjectReferenceLinkedRefsModel::_on_all_forms_renumbered() {
   auto size = this->_data.size();

   {
      auto tl = this->index(0, Column::RefFormID, {});
      auto br = this->index(size - 1, Column::RefFormID, {});
      emit dataChanged(tl, br);
   }
}

decltype(ObjectReferenceLinkedRefsModel::_data)::iterator ObjectReferenceLinkedRefsModel::_insertion_point_for(const Mapping& item) {
   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      item,
      [](const Mapping& a, const Mapping& b) {
         if (!a.keyword)
            return true;
         if (!b.keyword)
            return false;
         return a.cached.keyword.localeAwareCompare(b.cached.keyword) < 0;
      }
   );
}
void ObjectReferenceLinkedRefsModel::_re_sort_item(size_t from) {
   auto& list = this->_data;
   if (from >= list.size())
      return;
   bool moved     = false;
   auto editor_id = QString::fromStdString(list[from].keyword->editorID);
   cobb::vectors::re_sort_item_within(
      list,
      list.begin() + from,
      [editor_id](auto& a, auto& b) -> bool {
         if (!b.keyword)
            return false;
         return editor_id < QString::fromStdString(b.keyword->editorID);
      },
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

/*static*/ QString ObjectReferenceLinkedRefsModel::_name_of(const dovah::form_stub& stub) {
   if (dovah::form_type_is_reference(stub.form_type)) {
      if (stub.editorID.empty()) {
         if (auto* base = dovah::form_stub_helpers::get_base_form(&stub))
            return QString::fromStdString(base->editorID);
      }
   }
   return QString::fromStdString(stub.editorID);
}