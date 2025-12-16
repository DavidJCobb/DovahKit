#pragma once
#include <optional>
#include <QObject>
#include "dovah/data/story_manager.h"
#include "dovah/datastores/story_manager.h"
#include "helpers/singleton_ex.h"
#include "ui/types/logging/log_item.h"

class StoryManagerFormsModel;

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
         constexpr const StoryManagerFormsModel* model() const noexcept { return this->_model; }
         constexpr StoryManagerFormsModel* model() noexcept { return this->_model; }

         dovah::form_stub* containing_event_node_of(const dovah::form_stub&) const noexcept;
         std::optional<dovah::story_event_code::type> event_type_for(const dovah::form_stub& event_node) const noexcept;

         void move_node(const node& subject, const branch_node& dst_parent, const node* dst_previous);
         void move_node_within_parent(const node&, int by);
         bool delete_node(const node&);

      protected:
         datastore_type _datastore;
         StoryManagerFormsModel* _model = nullptr;
         struct {
            bool any_deletions_failed = false;
         } _handler_state;
   };
};