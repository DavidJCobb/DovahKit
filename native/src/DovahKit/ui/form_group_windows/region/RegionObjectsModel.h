#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include "dovah/forms/structs/region/generable_content/objects.h"
namespace dovah {
   namespace loaded_forms {
      class Region;
   }
   class form_stub;
}

// Qt often chokes on nested classes/structs
struct RegionObjectsModelObject {
   public:
      using object_params = dovah::loaded_forms::structs::region::generable_content::object_params;

   public:
      dovah::form_stub* form = nullptr;
      object_params     params;
};
Q_DECLARE_METATYPE(RegionObjectsModelObject);

class RegionObjectsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      using loaded_form_type        = dovah::loaded_forms::Region;
      using backend_collection_type = dovah::loaded_forms::structs::region::generable_content::raw_object_collection;
      using object_params           = dovah::loaded_forms::structs::region::generable_content::object_params;

      static constexpr const size_t ColumnCount = 1;

      using ObjectData = RegionObjectsModelObject;

      static constexpr const Qt::ItemDataRole ObjectDataRole = Qt::UserRole;

   protected:
      class Object : public ObjectData {
         public:
            ~Object();

         public:
            Object* parent = nullptr;
            std::vector<Object*> children;

            struct {
               QString editor_id;
            } cached;

         public:
            constexpr bool contains(const Object& o) const noexcept {
               for (auto* p = o.parent; p; p = p->parent)
                  if (p == this)
                     return true;
               return false;
            }
            constexpr size_t index_of(const Object& o) const noexcept {
               for (size_t i = 0; i < this->children.size(); ++i)
                  if (this->children[i] == &o)
                     return i;
               return (size_t)-1;
            }
      };
      std::vector<Object*> _data;

      #pragma region Node utils
         const Object* _node_for_qmi(const QModelIndex&) const;
         Object* _node_for_qmi(const QModelIndex&);
         QModelIndex _qmi_for_node(const Object&) const;
      #pragma endregion

      static bool allows_form_type(dovah::form_type);
      
   public:
      RegionObjectsModel(QObject* parent = nullptr);
      ~RegionObjectsModel();

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex&) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
            #pragma region Editing
               virtual bool removeRows(int row, int count, const QModelIndex& parent = {}) override;
            #pragma endregion
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex&, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex&) const override;
            virtual bool          setData(const QModelIndex&, const QVariant& value, int role) override;
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
         #pragma region Drag and drop
            #pragma region Whole-model queries
               virtual QStringList mimeTypes() const override;
               virtual Qt::DropActions supportedDragActions() const override;
               virtual Qt::DropActions supportedDropActions() const override;
            #pragma endregion
            virtual QMimeData* mimeData(const QModelIndexList&) const override;
            virtual bool canDropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) const override;
            virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) override;
         #pragma endregion
      #pragma endregion

      void importData(const backend_collection_type&);
      void exportData(backend_collection_type&, loaded_form_type&) const;
      void clear();

      QModelIndex insertObject(const QModelIndex& parent_qmi, int row, dovah::form_stub& base_form);
      QModelIndex insertObject(const QModelIndex& parent_qmi, int row, const ObjectData&);

   protected:
      void _on_form_deleted(dovah::form_stub&);
      void _on_form_modified(dovah::form_stub&);

      #pragma region Drag and drop implementation
         #pragma region Forms from the Object Window
            static bool _is_dragged_form_stub_list(const QMimeData&);
            bool _can_drop_form_stub_list(const QMimeData&, Qt::DropAction action) const;
            bool _drop_form_stub_list(const QMimeData&, Qt::DropAction action, const QModelIndex& parent_qmi, Object* parent_node, int row);
         #pragma endregion
         #pragma region Drag-moving nodes within our tree
            QMimeData* _get_node_drag_data(const QModelIndexList&) const;
            static bool _is_dragged_nodes(const QMimeData&);
            bool _can_drop_nodes(const QMimeData&, Qt::DropAction action, int row, const QModelIndex& parent_qmi) const;
            bool _drop_nodes(const QMimeData&, Qt::DropAction action, const QModelIndex& parent_qmi, Object* parent_node, int row);

            struct DragDropTracking {
               public:
                  using uid_t = uint64_t;

               public:
                  uid_t next_id = 0;
                  std::unordered_map<uid_t, Object*> nodes;

                  uid_t track(Object&);
                  void untrack(Object&);
                  void clear();
                  Object* get_by_id(uid_t);

                  void on_node_destroyed(Object&);
            };
            mutable DragDropTracking _drag_and_drop; // mutable because QAbstractItemModel::mimeData is const
         #pragma endregion
      #pragma endregion
};