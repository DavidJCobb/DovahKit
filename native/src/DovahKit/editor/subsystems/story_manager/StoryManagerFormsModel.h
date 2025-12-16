#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <QAbstractItemModel>
#include <QIcon>
#include <QString>
#include "dovah/data/story_manager.h"
namespace dovah {
   namespace datastores {
      namespace impl::story_manager {
         class node;
         class branch_node;
         class leaf_node;
      }
      class story_manager;
   }
   class form_stub;
}
namespace dovahkit::subsystems::story_manager {
   namespace passkeys {
      class core_controls_model;
   }
   class core;
}

class StoryManagerFormsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      static constexpr const Qt::ItemDataRole FormStubRole = Qt::UserRole;
      static constexpr const Qt::ItemDataRole QuestResetAfter24HoursRole = (Qt::ItemDataRole)(Qt::UserRole + 1);
      static constexpr const Qt::ItemDataRole QuestHoursUntilResetRole   = (Qt::ItemDataRole)(Qt::UserRole + 2);
      static constexpr const Qt::ItemDataRole EventTypeRole = (Qt::ItemDataRole)(Qt::UserRole + 3); // as an int

   protected:
      struct passkeys { // poor man's namespace
         passkeys() = delete;
         using core_controls_model = dovahkit::subsystems::story_manager::passkeys::core_controls_model;
      };

      using subsystem      = dovahkit::subsystems::story_manager::core;
      using datastore_type = dovah::datastores::story_manager;
      
      using node        = dovah::datastores::impl::story_manager::node;
      using branch_node = dovah::datastores::impl::story_manager::branch_node;
      using leaf_node   = dovah::datastores::impl::story_manager::leaf_node;

      // Nodes for quest forms are owned by this model.
      struct quest_node {
         bool operator==(const quest_node&) const noexcept = default;

         dovah::form_stub* stub = nullptr;
         QString editor_id;
         bool  reset_after_24_hours = false;
         float hours_until_reset    = 0.0F;
      };

      struct cached_event_data {
         dovah::story_event_code_t event = {};
      };
      struct cached_quest_data {
         uint32_t num_to_run     = 0;
         uint32_t max_concurrent = 0;
         std::vector<std::unique_ptr<quest_node>> quests;
      };
      //
      struct cached_node_data {
         QString editor_id;
         QString display_string;
         bool    is_random = false;
         bool    warn_if_none_started = false;
         std::variant<
            std::monostate,
            cached_event_data,
            cached_quest_data
         > typed;
      };

   public:
      StoryManagerFormsModel(passkeys::core_controls_model, QObject* parent = nullptr);

   protected:
      static const datastore_type& _get_datastore();

      #pragma region QMI-to-data mapping
         QModelIndex _qmi_for_node(const node&) const noexcept;

         const node* _node_for_qmi(const QModelIndex&) const noexcept;
         node* _node_for_qmi(const QModelIndex& qmi) noexcept { return const_cast<node*>(std::as_const(*this)._node_for_qmi(qmi)); }

         bool _qmi_is_quest_form(const QModelIndex&) const noexcept;
         const quest_node* _quest_for_qmi(const QModelIndex&) const noexcept;
         quest_node* _quest_for_qmi(const QModelIndex& qmi) noexcept { return const_cast<quest_node*>(std::as_const(*this)._quest_for_qmi(qmi)); }
      #pragma endregion
      #pragma region Caching
         const cached_node_data* _get_cached_data(const node&) const noexcept;
         const cached_quest_data* _get_cached_quest_data(const node&) const noexcept;

         void _recache_node_core_properties(const node&);
         void _recache_quest_data(const node&, cached_node_data&);
         void _recache_quest_list(const node&, cached_node_data&, bool clobber_sans_signals);
         void _recache_node_from_scratch(const node&);
         void _recache_node(const node&);
      #pragma endregion

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent_qmi) const override;
            virtual int columnCount(const QModelIndex&) const override;
            virtual int rowCount(const QModelIndex&) const override;
            virtual QModelIndex parent(const QModelIndex&) const override;
         #pragma endregion
         virtual QVariant data(const QModelIndex&, int role) const override;
         virtual Qt::ItemFlags flags(const QModelIndex&) const override;

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      QModelIndex index(const dovah::form_stub&) const noexcept;

   public: // passkeyed
      #pragma region Puppeteering by subsystem core
         void _on_before_datastore_reset(passkeys::core_controls_model);
         void _on_after_datastore_reset(passkeys::core_controls_model, bool rebuilt);

         void _on_node_deletion_imminent(passkeys::core_controls_model, const node&);
         void _on_node_deletion_complete(passkeys::core_controls_model);

         void _on_node_placement_imminent(passkeys::core_controls_model, const node& subject, const branch_node& dst_parent, size_t dst_pos);
         void _on_node_placement_complete(passkeys::core_controls_model, const node&);

         void _on_form_modified(passkeys::core_controls_model, const dovah::form_stub&);
      #pragma endregion

   protected:
      void _make_icons();

   protected:
      struct {
         std::unordered_map<node*, cached_node_data> nodes;
      } _cache;
      struct {
         QIcon event;
         QIcon quest_form;
         QIcon quest_list;
         QIcon branch;
      } _icons;
      struct {
         bool last_placement_was_insertion = false;
      } _callback_state;
};