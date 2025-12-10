#pragma once
#include <exception>
#include <unordered_map>
#include <vector>
#include <QAbstractItemModel>
#include "dovah/datastores/camera_paths.h"
namespace dovah {
   class form_stub;
}

class CameraPathFormsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      static constexpr const Qt::ItemDataRole FormStubRole = (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole);

      static constexpr const size_t ColumnCount = 1;

      class too_many_to_duplicate_exception : public std::exception {
         public:
            too_many_to_duplicate_exception(size_t needed, size_t available)
            :
               std::exception("Insufficient form IDs available to duplicate this camera path"),
               form_id_counts({needed, available})
            {};

            const struct {
               size_t needed;
               size_t available;
            } form_id_counts;
      };

   protected:
      using datastore_type = dovah::datastores::camera_paths;
      using datastore_item = datastore_type::node_parent;
      using datastore_node = datastore_type::node;

      struct node_cached_data {
         QString display_string;
      };

      #pragma region Node/QMI utils and node lookups
         QModelIndex _qmi_for_model_root() const;
         QModelIndex _qmi_for_item(const datastore_item&) const;
         QModelIndex _qmi_for_node(const datastore_node&, int column = 0) const;
         QModelIndex _qmi_for_child_node(int row, int column, const datastore_item& parent) const;
         bool _qmi_is_child_of(const QModelIndex&, const datastore_item&) const;

         const datastore_node* _child_node_by_row(const datastore_item& parent, size_t row) const;

         const datastore_item* _item_for_qmi(const QModelIndex&) const;
         datastore_item* _item_for_qmi(const QModelIndex&);
         const datastore_node* _node_for_qmi(const QModelIndex&) const;
         datastore_node* _node_for_qmi(const QModelIndex&);

         const datastore_node* _child_node_for_qmi(const QModelIndex& parent_qmi, int row) const;
         datastore_node* _child_node_for_qmi(const QModelIndex& parent_qmi, int row);
      #pragma endregion

      #pragma region Form events
         void _on_game_data_acquired();
         void _on_game_data_abandon();
         void _on_form_created(dovah::form_stub&);
         void _on_form_modified(dovah::form_stub&);
         void _on_form_deletion_imminent(dovah::form_stub&, bool just_being_flagged);
         void _on_form_deleted(uint32_t form_id, bool just_being_flagged);
      #pragma endregion

      void _rebuild_datastore();
      void _recache(const datastore_node&);
      void _recache(const dovah::form_stub&);

      #pragma region Form utils
         dovah::form_stub* _try_silently_create_camera_path(QString editor_id) noexcept(false);
         dovah::form_stub* _try_silently_duplicate_camera_path(dovah::form_stub& idle) noexcept(false);
      #pragma endregion
      #pragma region Node utils
         bool _can_move(const datastore_node& subject, const datastore_item& destination) const;
         void _unchecked_move(datastore_node& subject, datastore_item& destination, int row = -1);
      #pragma endregion

   public:
      CameraPathFormsModel(QObject* parent = nullptr);

      #pragma region Accessors
         QModelIndex index(dovah::form_stub&) const noexcept;

         QModelIndex createCameraPath(const QModelIndex& parent, QString editor_id) noexcept(false); // may throw `dovah::exceptions::form_creation_failed`
         QModelIndex createCameraPathAfter(const QModelIndex& previous_sibling, QString editor_id) noexcept(false); // may throw `dovah::exceptions::form_creation_failed`

         QModelIndex duplicateCameraPath(const QModelIndex&, bool and_descendants) noexcept(false); // may throw `too_many_to_duplicate_exception` or `dovah::exceptions::form_creation_failed`

         void deleteCameraPath(const QModelIndex&, QWidget* error_dialog_parent = nullptr); // deletes the idle *and its descendants*

         bool canMoveUp(const QModelIndex&) const;
         bool canMoveDown(const QModelIndex&) const;
         void moveUp(const QModelIndex&);
         void moveDown(const QModelIndex&);
      #pragma endregion

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
         #pragma region Drag and drop
            #pragma region Whole-model queries
               virtual QStringList mimeTypes() const override;
               virtual Qt::DropActions supportedDropActions() const override;
            #pragma endregion
            virtual QMimeData* mimeData(const QModelIndexList&) const override;
            virtual bool canDropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) const override;
            virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) override;
         #pragma endregion
      #pragma endregion

   protected:
      datastore_type _datastore;
      std::unordered_map<datastore_node*, node_cached_data> _cache;
      struct {
         bool ignore_next_created_camera_path = false;
         bool emitted_last_deletion = false;
         bool last_node_placement_was_an_insertion = false;
      } _callback_state;
      struct {
         bool any_deletions_failed = false;
      } _handler_state;

      struct DragDropTracking {
         public:
            using uid_t = uint64_t;

         public:
            uid_t next_id = 0;
            std::unordered_map<uid_t, datastore_node*> nodes;

            uid_t track(datastore_node&);
            void untrack(datastore_node&);
            void clear();
            datastore_node* get_by_id(uid_t);
      };
      mutable DragDropTracking _drag_and_drop; // mutable because QAbstractItemModel::mimeData is const
};