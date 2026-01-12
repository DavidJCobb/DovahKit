#include "./ObjectReferenceReflectedObjectsModel.h"
#include "helpers/vectors/re_sort_item_within.h"
#include "dovah/forms/components/extra_data/types/r/reflector_refs.h"
#include "dovah/forms/ObjectReference.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "dovah/form_stubs/helpers/is_used_by.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"

ObjectReferenceReflectedObjectsModel::ObjectReferenceReflectedObjectsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ObjectReferenceReflectedObjectsModel::_on_data_abandoned);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formRenumbered, this, [this](dovah::form_stub* stub) { this->_on_form_renumbered(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &ObjectReferenceReflectedObjectsModel::_on_all_forms_renumbered);
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex ObjectReferenceReflectedObjectsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex ObjectReferenceReflectedObjectsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex ObjectReferenceReflectedObjectsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int ObjectReferenceReflectedObjectsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int ObjectReferenceReflectedObjectsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant ObjectReferenceReflectedObjectsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_data.size())
            return {};
         auto& src = this->_data[index.row()];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (index.column()) {
                  case Column::RefName:
                     return src.cached.ref;
                  case Column::RefFormID:
                     if (!src.ref)
                        break;
                     return editor_helpers::form_id_to_string(src.ref->formID);
                  case Column::Type:
                     switch (src.type) {
                        case RowType::Both:
                           return tr("Reflection and refraction");
                        case RowType::Reflection:
                           return tr("Reflection");
                        case RowType::Refraction:
                           return tr("Refraction");
                     }
                     break;
               }
               break;
            case FormStubRole:
               switch (index.column()) {
                  case Column::RefName:
                  case Column::RefFormID:
                     return QVariant::fromValue(src.ref);
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags ObjectReferenceReflectedObjectsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant ObjectReferenceReflectedObjectsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      switch (section) {
         case Column::RefName:
            return tr("Reflects ref");
         case Column::RefFormID:
            return tr("Ref ID");
      }
      return {};
   }
#pragma endregion

void ObjectReferenceReflectedObjectsModel::setSubject(dovah::form_stub* ref) {
   this->beginResetModel();
   this->_data.clear();
   this->_subject = ref;
   if (!ref) {
      this->endResetModel();
      return;
   }

   std::unordered_map<dovah::form_stub*, RowType> results;
   for (const auto& [_, use] : ref->inbound) {
      auto* user = use.other;
      assert(!!user);
      if (user == ref) // a ref shouldn't reflect itself, so don't even bother checking self-uses
         continue;
      if (!dovah::form_type_is_reference(user->form_type))
         continue;
      auto loaded = user->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
      if (!loaded)
         continue;
      auto* extra = loaded->extra_data.get<extra_data_type>();
      if (!extra)
         continue;
      results[user] = _reflection_type_via(*extra);
   }
   for (auto& [user, type] : results) {
      Row row;
      row.ref  = user;
      row.type = type;
      row.cached.ref = _name_of(*user);
      this->_data.push_back(std::move(row));
   }
   results.clear();

   std::sort(
      this->_data.begin(),
      this->_data.end(),
      _sort_comparator
   );

   this->endResetModel();
}

void ObjectReferenceReflectedObjectsModel::_on_data_abandoned() {
   this->beginResetModel();
   this->_data.clear();
   this->_subject = nullptr;
   this->endResetModel();
}
void ObjectReferenceReflectedObjectsModel::_on_form_deleted(dovah::form_stub& stub) {
   if (this->_subject == nullptr) {
      assert(this->_data.empty());
      return;
   }
   if (this->_subject == &stub) {
      this->setSubject(nullptr);
      return;
   }
   if (dovah::form_type_is_reference(stub.form_type)) {
      _remove_reflection_of(stub);
      return;
   }
}
void ObjectReferenceReflectedObjectsModel::_on_form_modified(dovah::form_stub& stub) {
   if (this->_subject == nullptr) {
      assert(this->_data.empty());
      return;
   }
   if (!dovah::form_type_is_reference(stub.form_type))
      return;

   if (!dovah::form_stub_helpers::is_used_by(*this->_subject, stub)) {
      _remove_reflection_of(stub);
      return;
   }
   auto loaded = stub.load().ptr_cast<dovah::loaded_forms::ObjectReference>();
   if (!loaded) {
      _remove_reflection_of(stub);
      return;
   }
   const auto* extra = loaded->extra_data.get<extra_data_type>();
   if (!extra) {
      _remove_reflection_of(stub);
      return;
   }

   _update_reflection_of(stub, _reflection_type_via(*extra));
}
void ObjectReferenceReflectedObjectsModel::_on_form_renumbered(dovah::form_stub& stub) {
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
void ObjectReferenceReflectedObjectsModel::_on_all_forms_renumbered() {
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

ObjectReferenceReflectedObjectsModel::RowType ObjectReferenceReflectedObjectsModel::_reflection_type_via(const extra_data_type& extra) {
   RowType type = RowType::None;
   for (auto& entry : extra.entries) {
      if (entry.target == this->_subject) {
         switch (entry.type) {
            case extra_data_type::type::refraction:
               if (type == RowType::Reflection)
                  return RowType::Both;
               type = RowType::Refraction;
               break;
            case extra_data_type::type::reflection:
               if (type == RowType::Refraction)
                  return RowType::Both;
               type = RowType::Refraction;
               break;
         }
      }
   }
   return type;
}
void ObjectReferenceReflectedObjectsModel::_remove_reflection_of(dovah::form_stub& stub) {
   auto&  list = this->_data;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto& item = list[i];
      if (item.ref == &stub) {
         this->beginRemoveRows({}, i, i);
         list.erase(list.begin() + i);
         this->endRemoveRows();
         return;
      }
   }
}
void ObjectReferenceReflectedObjectsModel::_update_reflection_of(dovah::form_stub& stub, RowType type) {
   if (type == RowType::None) {
      _remove_reflection_of(stub);
      return;
   }

   for (size_t i = 0; i < this->_data.size(); ++i) {
      if (this->_data[i].ref == &stub) {
         this->_data[i].type = type;
         auto qmi = this->index(i, Column::RefName, {});
         emit dataChanged(qmi, qmi);
         return;
      }
   }

   Row row;
   row.ref  = &stub;
   row.type = type;
   row.cached.ref = _name_of(stub);
   //
   auto it = _insertion_point_for(row);
   auto i  = std::distance(this->_data.begin(), it);
   this->beginInsertRows({}, i, i);
   this->_data.insert(it, std::move(row));
   this->endInsertRows();
}

decltype(ObjectReferenceReflectedObjectsModel::_data)::iterator ObjectReferenceReflectedObjectsModel::_insertion_point_for(const Row& item) {
   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      item,
      _sort_comparator
   );
}
void ObjectReferenceReflectedObjectsModel::_re_sort_item(size_t from) {
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
/*static*/ bool ObjectReferenceReflectedObjectsModel::_sort_comparator(const Row& a, const Row& b) {
   return a.cached.ref.localeAwareCompare(b.cached.ref) < 0;
}

/*static*/ QString ObjectReferenceReflectedObjectsModel::_name_of(const dovah::form_stub& stub) {
   if (dovah::form_type_is_reference(stub.form_type)) {
      if (stub.editorID.empty()) {
         if (auto* base = dovah::form_stub_helpers::get_base_form(stub))
            return QString::fromStdString(base->editorID);
      }
   }
   return QString::fromStdString(stub.editorID);
}