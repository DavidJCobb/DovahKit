#pragma once
#include <QObject>
#include "dovah/datastores/story_manager.h"
#include "helpers/singleton_ex.h"
#include "ui/types/logging/log_item.h"

namespace dovahkit::subsystems::story_manager {
   class core;

   class core : public QObject, public cobb::singleton_ex<core> {
      Q_OBJECT;
      public:
         using datastore_type = dovah::datastores::story_manager;

         using node        = datastore_type::node;
         using branch_node = datastore_type::branch_node;
         using leaf_node   = datastore_type::leaf_node;

      protected:
         core();
         ~core();

      public:
         using singleton_ex::get;
         using singleton_ex::get_or_create;

      protected:
         void _rebuild_datastore();

      public:
         constexpr const datastore_type& datastore() const noexcept { return this->_datastore; }

         void move_node(const node& subject, const branch_node& dst_parent, const node* dst_previous);
         void move_node_within_parent(const node&, int by);
         bool delete_node(const node&);

      signals:
         void nodeDeletionImminent(const node&);
         void nodeDeletionComplete();
         void nodePlacementImminent(const node&, const branch_node& dst_parent, size_t at);
         void nodePlacementComplete(const node&);
         void resetImminent();
         void resetComplete();

      protected:
         datastore_type _datastore;
         struct {
            bool any_deletions_failed = false;
         } _handler_state;
   };
};