#include "./ObjectReferenceReflectingWaterModel.h"
#include <QComboBox>
#include "helpers/vectors/re_sort_item_within.h"
#include "dovah/forms/components/extra_data/types/l/lit_water.h"
#include "dovah/forms/components/extra_data/types/r/reflector_refs.h"
#include "dovah/forms/ObjectReference.h"
#include "dovah/form_stubs/helpers/get_activator_water_type.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "dovah/form_stubs/helpers/is_used_by.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}

#pragma region ReflectionTypeItemDelegate
   QWidget* ObjectReferenceReflectingWaterModel::ReflectionTypeItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
      QComboBox* widget = new QComboBox(parent);
      widget->addItem(tr("Reflection"), (int)RowType::Reflection);
      widget->addItem(tr("Refraction"), (int)RowType::Refraction);
      widget->addItem(tr("Refraction and refraction"), (int)RowType::Both);
      return widget;
   }
   void ObjectReferenceReflectingWaterModel::ReflectionTypeItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
      QComboBox* widget = qobject_cast<QComboBox*>(editor);
      assert(!!widget);
      const auto v = index.data(Qt::EditRole).toInt();
      const int  i = widget->findData(v);
      if (i >= 0)
         widget->setCurrentIndex(i);
   }
   void ObjectReferenceReflectingWaterModel::ReflectionTypeItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
      QComboBox* widget = qobject_cast<QComboBox*>(editor);
      assert(!!widget);
      model->setData(index, widget->currentData().toInt(), Qt::EditRole);
   }
#pragma endregion

ObjectReferenceReflectingWaterModel::ObjectReferenceReflectingWaterModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ObjectReferenceReflectingWaterModel::_on_data_abandoned);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formRenumbered, this, [this](dovah::form_stub* stub) { this->_on_form_renumbered(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &ObjectReferenceReflectingWaterModel::_on_all_forms_renumbered);
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex ObjectReferenceReflectingWaterModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= columnCount({}))
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex ObjectReferenceReflectingWaterModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex ObjectReferenceReflectingWaterModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int ObjectReferenceReflectingWaterModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int ObjectReferenceReflectingWaterModel::columnCount(const QModelIndex& parent) const /*override*/ {
         if (this->_context.type == ExtraDataType::ReflectorRefs) {
            return 3;
         }
         return 2;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant ObjectReferenceReflectingWaterModel::data(const QModelIndex& index, int role) const /*override*/ {
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
            case Qt::EditRole:
               if (index.column() == Column::Type) {
                  return (int)src.type;
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
      /*virtual*/ Qt::ItemFlags ObjectReferenceReflectingWaterModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
      /*virtual*/ bool ObjectReferenceReflectingWaterModel::setData(const QModelIndex& index, const QVariant& value, int role) /*override*/ {
         if (!index.isValid())
            return false;
         if (index.row() >= this->_data.size())
            return false;
         auto& src = this->_data[index.row()];
         if (role == Qt::EditRole && index.column() == Column::Type) {
            if (this->_context.type != ExtraDataType::ReflectorRefs)
               return false;
            src.type = (RowType)value.toInt();
            return true;
         }
         return false;
      }
   #pragma endregion
   /*virtual*/ QVariant ObjectReferenceReflectingWaterModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      switch (section) {
         case Column::RefName:
            return tr("Placed water");
         case Column::RefFormID:
            return tr("ID");
         case Column::Type:
            return tr("Type");
      }
      return {};
   }
#pragma endregion

void ObjectReferenceReflectingWaterModel::import_data(ExtraDataType type, backend_form_type& ref) {
   this->beginResetModel();
   this->_context = {
      .ref  = &ref.stub,
      .type = type,
   };

   switch (type) {
      case ExtraDataType::LitWater:
         {
            auto* extra = ref.extra_data.get<extra_data_types::lit_water>();
            if (!extra)
               break;
            for (auto& src_item : extra->refs) {
               if (!src_item)
                  continue;
               if (!is_valid_reflector(*src_item.get_form_stub()))
                  continue;
               auto& dst_item = this->_data.emplace_back();
               dst_item.ref = src_item.get_form_stub();
               dst_item.cached.ref = _name_of(*dst_item.ref);
            }
         }
         break;
      case ExtraDataType::ReflectorRefs:
         {
            auto* extra = ref.extra_data.get<extra_data_types::reflector_refs>();
            if (!extra)
               break;
            for (auto& src_item : extra->entries) {
               if (!src_item.target)
                  continue;
               if (!is_valid_reflector(*src_item.target.get_form_stub()))
                  continue;
               bool exists = false;
               for (auto& dst_item : this->_data) {
                  if (dst_item.ref != src_item.target.get_form_stub())
                     continue;
                  exists = true;
                  switch (src_item.type) {
                     using enum extra_data_types::reflector_refs::type;
                     case reflection:
                        if (dst_item.type == RowType::Refraction)
                           dst_item.type = RowType::Both;
                        break;
                     case refraction:
                        if (dst_item.type == RowType::Reflection)
                           dst_item.type = RowType::Both;
                        break;
                  }
                  break;
               }
               if (exists)
                  continue;
               auto& dst_item = this->_data.emplace_back();
               dst_item.ref = src_item.target.get_form_stub();
               dst_item.cached.ref = _name_of(*dst_item.ref);
               switch (src_item.type) {
                  using enum extra_data_types::reflector_refs::type;
                  case reflection:
                     dst_item.type = RowType::Reflection;
                     break;
                  case refraction:
                     dst_item.type = RowType::Refraction;
                     break;
               }
            }
         }
         break;
   }

   std::sort(
      this->_data.begin(),
      this->_data.end(),
      _sort_comparator
   );

   this->endResetModel();
}
void ObjectReferenceReflectingWaterModel::export_data(backend_form_type& ref) {
   switch (this->_context.type) {
      case ExtraDataType::LitWater:
         if (this->_data.empty()) {
            ref.extra_data.remove<extra_data_types::lit_water>(ref);
         } else {
            auto* extra = ref.extra_data.get_or_create<extra_data_types::lit_water>();
            extra->clear_contained_formIDs(ref);
            extra->refs.reserve(this->_data.size());
            for (auto& src_item : this->_data) {
               extra->refs.emplace_back().set(ref, src_item.ref);
            }
         }
         break;
      case ExtraDataType::ReflectorRefs:
         if (this->_data.empty()) {
            ref.extra_data.remove<extra_data_types::reflector_refs>(ref);
         } else {
            auto* extra = ref.extra_data.get_or_create<extra_data_types::reflector_refs>();
            extra->clear_contained_formIDs(ref);
            for (auto& src_item : this->_data) {
               auto& dst_item = extra->entries.emplace_back();
               dst_item.target.set(ref, src_item.ref);
               switch (src_item.type) {
                  case RowType::Reflection:
                     dst_item.type = extra_data_types::reflector_refs::type::reflection;
                     break;
                  case RowType::Refraction:
                     dst_item.type = extra_data_types::reflector_refs::type::refraction;
                     break;
                  case RowType::Both:
                     dst_item.type = extra_data_types::reflector_refs::type::reflection;
                     {
                        auto& other = extra->entries.emplace_back();
                        other.target.set(ref, src_item.ref);
                        other.type = extra_data_types::reflector_refs::type::refraction;
                     }
                     break;
               }
            }
         }
         break;
   }
}

void ObjectReferenceReflectingWaterModel::add_reflector(dovah::form_stub& ref) {
   if (!is_valid_reflector(ref))
      return;
   for (auto& item : this->_data)
      if (item.ref == &ref)
         return;

   Row row;
   row.ref = &ref;
   if (this->_context.type == ExtraDataType::ReflectorRefs)
      row.type = RowType::Reflection;
   row.cached.ref = _name_of(ref);
   //
   auto it = _insertion_point_for(row);
   auto i  = std::distance(this->_data.begin(), it);
   this->beginInsertRows({}, i, i);
   this->_data.insert(it, std::move(row));
   this->endInsertRows();
}
void ObjectReferenceReflectingWaterModel::remove_reflector(dovah::form_stub& ref) {
   if (!dovah::form_type_is_reference(ref.form_type))
      return;
   for (size_t i = 0; i < this->_data.size(); ++i) {
      if (this->_data[i].ref != &ref)
         continue;
      this->beginRemoveRows({}, i, i);
      this->_data.erase(this->_data.begin() + i);
      this->endRemoveRows();
      return;
   }
}
bool ObjectReferenceReflectingWaterModel::is_valid_reflector(dovah::form_stub& ref) const {
   if (&ref == this->_context.ref)
      return false;
   return is_a_reflector_at_all(ref);
}
bool ObjectReferenceReflectingWaterModel::is_a_reflector_at_all(dovah::form_stub& ref) {
   if (!dovah::form_type_is_reference(ref.form_type))
      return false;

   auto* base = dovah::form_stub_helpers::get_base_form(ref);
   if (!base)
      return false;
   if (!dovah::form_stub_helpers::get_activator_water_type(*base))
      return false;

   return true;
}

void ObjectReferenceReflectingWaterModel::_on_data_abandoned() {
   this->beginResetModel();
   this->_data.clear();
   this->_context = {};
   this->endResetModel();
}
void ObjectReferenceReflectingWaterModel::_on_form_deleted(dovah::form_stub& stub) {
   if (this->_context.ref == nullptr) {
      assert(this->_data.empty());
      return;
   }
   if (this->_context.ref == &stub) {
      this->beginResetModel();
      this->_data.clear();
      this->_context = {};
      this->endResetModel();
      return;
   }
   if (dovah::form_type_is_reference(stub.form_type)) {
      _remove_reflection_of(stub);
      return;
   }
}
void ObjectReferenceReflectingWaterModel::_on_form_modified(dovah::form_stub& stub) {
   for (size_t i = 0; i < this->_data.size(); ++i) {
      if (this->_data[i].ref == &stub) {
         auto qmi = this->index(i, Column::RefName, {});
         emit dataChanged(qmi, qmi);
         this->_re_sort_item(i);
         return;
      }
   }
}
void ObjectReferenceReflectingWaterModel::_on_form_renumbered(dovah::form_stub& stub) {
   if (this->_context.ref == nullptr) {
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
void ObjectReferenceReflectingWaterModel::_on_all_forms_renumbered() {
   if (this->_context.ref == nullptr) {
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

void ObjectReferenceReflectingWaterModel::_remove_reflection_of(dovah::form_stub& stub) {
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

decltype(ObjectReferenceReflectingWaterModel::_data)::iterator ObjectReferenceReflectingWaterModel::_insertion_point_for(const Row& item) {
   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      item,
      _sort_comparator
   );
}
void ObjectReferenceReflectingWaterModel::_re_sort_item(size_t from) {
   auto& list = this->_data;
   if (from >= list.size())
      return;
   bool moved = false;
   cobb::vectors::re_sort_item_within(
      list,
      list.begin() + from,
      _sort_comparator,
      [&moved, this, &list](decltype(_data)::iterator from_it, decltype(_data)::iterator to_it) {
         size_t from  = std::distance(list.begin(), from_it);
         size_t to    = std::distance(list.begin(), to_it);
         moved = this->beginMoveRows(
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
/*static*/ bool ObjectReferenceReflectingWaterModel::_sort_comparator(const Row& a, const Row& b) {
   return a.cached.ref.localeAwareCompare(b.cached.ref) < 0;
}

/*static*/ QString ObjectReferenceReflectingWaterModel::_name_of(const dovah::form_stub& stub) {
   if (dovah::form_type_is_reference(stub.form_type)) {
      if (stub.editorID.empty()) {
         if (auto* base = dovah::form_stub_helpers::get_base_form(stub))
            return QString::fromStdString(base->editorID);
      }
   }
   return QString::fromStdString(stub.editorID);
}