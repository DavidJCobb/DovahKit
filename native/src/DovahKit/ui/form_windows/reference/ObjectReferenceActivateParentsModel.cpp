#include "./ObjectReferenceActivateParentsModel.h"
#include "dovah/forms/components/extra_data/types/a/activate_parents.h"
#include "dovah/forms/ObjectReference.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"

ObjectReferenceActivateParentsModel::ObjectReferenceActivateParentsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ObjectReferenceActivateParentsModel::_on_data_abandoned);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formRenumbered, this, [this](dovah::form_stub* stub) { this->_on_form_renumbered(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &ObjectReferenceActivateParentsModel::_on_all_forms_renumbered);
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex ObjectReferenceActivateParentsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex ObjectReferenceActivateParentsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex ObjectReferenceActivateParentsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int ObjectReferenceActivateParentsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int ObjectReferenceActivateParentsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant ObjectReferenceActivateParentsModel::data(const QModelIndex& index, int role) const /*override*/ {
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
                  case Column::Delay:
                     return src.delay;
               }
               break;
            case FormStubRole:
               return QVariant::fromValue(src.ref);
            case DelayRole:
               return src.delay;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags ObjectReferenceActivateParentsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant ObjectReferenceActivateParentsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      switch (section) {
         case Column::RefName:
            return tr("Activate Parent");
         case Column::RefFormID:
            return tr("ID");
         case Column::Delay:
            return tr("Delay");
      }
      return {};
   }
   #pragma region Editing
      /*virtual*/ bool ObjectReferenceActivateParentsModel::removeRows(int row, int count, const QModelIndex& parent) /*override*/ {
         if (row < 0 || count <= 0)
            return false;
         if (row + count >= this->_data.size())
            return false;
         if (parent.isValid()) // items can't have children
            return false;
         this->beginRemoveRows(parent, row, row + count - 1);
         auto bit = this->_data.begin() + row;
         auto eit = bit + count;
         this->_data.erase(bit, eit);
         this->endRemoveRows();
         return true;
      }
   #pragma endregion
#pragma endregion

void ObjectReferenceActivateParentsModel::importData(const backend_form_type& form) {
   this->beginResetModel();
   this->_data.clear();
   if (auto* extra = form.extra_data.get<extra_data_type>()) {
      for (auto& link : extra->parents) {
         if (!link.ref)
            continue;
         bool already_present = false;
         for (auto& item : this->_data) {
            if (item.ref == link.ref.get_form_stub()) {
               already_present = true;
               item.ref = link.ref.get_form_stub();
               break;
            }
         }
         if (!already_present) {
            auto& item = this->_data.emplace_back();
            item.delay = link.delay;
            item.ref   = link.ref.get_form_stub();
            if (item.ref) {
               item.cached.ref = _name_of(*item.ref);
            }
         }
      }
   }
   this->endResetModel();
}
void ObjectReferenceActivateParentsModel::exportData(backend_form_type& form) const {
   if (this->_data.empty()) {
      form.extra_data.remove<extra_data_type>(form);
      return;
   }
   auto* extra = form.extra_data.get_or_create<extra_data_type>();
   extra->clear_contained_formIDs(form);
   for (auto& src : this->_data) {
      auto& dst = extra->parents.emplace_back();
      dst.ref.set(form, src.ref);
      dst.delay = src.delay;
   }
}

QModelIndex ObjectReferenceActivateParentsModel::setRefDelay(dovah::form_stub& ref, float delay) {
   if (!dovah::form_type_is_reference(ref.form_type))
      return {};
   for (size_t i = 0; i < this->_data.size(); ++i) {
      if (this->_data[i].ref == &ref) {
         this->_data[i].delay = delay;
         auto tl = this->index(i, 0, {});
         auto br = this->index(i, ColumnCount - 1, {});
         emit dataChanged(tl, br);
         return tl;
      }
   }
   size_t i = this->_data.size();
   this->beginInsertRows({}, i, i);
   auto& item = this->_data.emplace_back();
   item.ref   = &ref;
   item.delay = delay;
   this->endInsertRows();
   return this->index(i, 0, {});
}
void ObjectReferenceActivateParentsModel::setRow(size_t i, dovah::form_stub& ref, float delay) {
   if (i >= this->_data.size())
      return;

   this->_data[i].ref   = &ref;
   this->_data[i].delay = delay;

   emit dataChanged(this->index(i, 0, {}), this->index(i, ColumnCount - 1, {}));
}

std::vector<dovah::form_stub*> ObjectReferenceActivateParentsModel::allRefs() const {
   std::vector<dovah::form_stub*> out;
   out.reserve(this->_data.size());
   for (auto& item : this->_data)
      out.push_back(item.ref);
   return out;
}
bool ObjectReferenceActivateParentsModel::containsRef(const dovah::form_stub& ref) const {
   for (auto& item : this->_data)
      if (item.ref == &ref)
         return true;
   return false;
}

void ObjectReferenceActivateParentsModel::_on_data_abandoned() {
   this->beginResetModel();
   this->_data.clear();
   this->endResetModel();
}
void ObjectReferenceActivateParentsModel::_on_form_deleted(dovah::form_stub& stub) {
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
void ObjectReferenceActivateParentsModel::_on_form_modified(dovah::form_stub& stub) {
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
void ObjectReferenceActivateParentsModel::_on_form_renumbered(dovah::form_stub& stub) {
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
void ObjectReferenceActivateParentsModel::_on_all_forms_renumbered() {
   auto size = this->_data.size();
   {
      auto tl = this->index(0, Column::RefFormID, {});
      auto br = this->index(size - 1, Column::RefFormID, {});
      emit dataChanged(tl, br);
   }
}

/*static*/ QString ObjectReferenceActivateParentsModel::_name_of(const dovah::form_stub& stub) {
   if (dovah::form_type_is_reference(stub.form_type)) {
      if (stub.editorID.empty()) {
         if (auto* base = dovah::form_stub_helpers::get_base_form(stub))
            return QString::fromStdString(base->editorID);
      }
   }
   return QString::fromStdString(stub.editorID);
}