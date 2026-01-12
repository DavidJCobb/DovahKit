#pragma once
#include <cstdint>
#include <unordered_map>
#include <QAbstractItemModel>
#include "ui/model_utils/drag_drop_node_id_map.h"
#include "ui/types/regions/generable_content/object_collection.h"
namespace dovah {
   class form_stub;
}
namespace ui::types::regions {
   class region;
}

class RegionObjectsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      using frontend_form_data = ui::types::regions::region;

      using tree_type = ui::types::regions::generable_content::object_collection;
      using node_type = tree_type::object;

      using object_data = ui::types::regions::generable_content::object_data;

      static constexpr const size_t ColumnCount = 1;

      static constexpr const Qt::ItemDataRole ObjectDataRole = Qt::UserRole;

   protected:
      struct node_cached_data {
         QString editor_id;
      };

      tree_type _tree;
      std::unordered_map<node_type*, node_cached_data> _cache;

      #pragma region Node utils
         const node_type* _node_for_qmi(const QModelIndex&) const;
         node_type* _node_for_qmi(const QModelIndex&);
         QModelIndex _qmi_for_node(const node_type&) const;
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

      void importData(const frontend_form_data&);
      void exportData(frontend_form_data&) const;
      void clear();

      QModelIndex insertObject(const QModelIndex& parent_qmi, int row, dovah::form_stub& base_form);
      QModelIndex insertObject(const QModelIndex& parent_qmi, int row, const object_data&);

      // We intentionally do not implement QAbstractItemModel::removeRow(s), to avoid bad jank 
      // involving internal-move drag-and-drop operations as managed by QAbstractItemView. Those 
      // operations are implemented as "remove from source, insert copy into destination" which 
      // isn't terribly great for us.
      //
      // These functions are provided as alternatives.
      void removeObject(const QModelIndex&);
      void removeObjects(const QModelIndex& parent_qmi, size_t row, size_t count);

   protected:
      void _on_node_destroyed(node_type&);

      void _on_form_deleted(dovah::form_stub&);
      void _on_form_modified(dovah::form_stub&);

      void _recache_node(node_type&, bool silent = false);

      #pragma region Drag and drop implementation
         #pragma region Forms from the Object Window
            static bool _is_dragged_form_stub_list(const QMimeData&);
            bool _can_drop_form_stub_list(const QMimeData&, Qt::DropAction action) const;
            bool _drop_form_stub_list(const QMimeData&, Qt::DropAction action, const QModelIndex& parent_qmi, node_type* parent_node, int row);
         #pragma endregion
         #pragma region Drag-moving nodes within our tree
            QMimeData* _get_node_drag_data(const QModelIndexList&) const;
            static bool _is_dragged_nodes(const QMimeData&);
            bool _can_drop_nodes(const QMimeData&, Qt::DropAction action, int row, const QModelIndex& parent_qmi) const;
            bool _drop_nodes(const QMimeData&, Qt::DropAction action, const QModelIndex& parent_qmi, node_type* parent_node, int row);

            mutable ui::model_utils::drag_drop_node_id_map<node_type> _drag_and_drop; // mutable because QAbstractItemModel::mimeData is const
         #pragma endregion
      #pragma endregion
};