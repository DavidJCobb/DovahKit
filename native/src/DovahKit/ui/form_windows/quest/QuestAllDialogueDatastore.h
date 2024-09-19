#pragma once
#include <cstdint>
#include <vector>
#include <QObject>
#include <QString>
#include "dovah/data/dialogue/category.h"

namespace dovah {
   namespace dialogue {
      struct topic_subtype;
   }
   class form_stub;
}

class QuestAllDialogueDatastore : public QObject {
   Q_OBJECT;
   public:
      QuestAllDialogueDatastore(QObject* parent = nullptr);
      ~QuestAllDialogueDatastore();

   public:
      struct Branch;
      struct Topic;
      struct Info;

      struct Info {
         Topic* parent = nullptr;

         dovah::form_stub* stub = nullptr;
         bool deleted = false;
         struct {
            QString  editor_id;
            size_t   response_count = 0;
            QString  responses;
            QString  responses_tooltip;
            QString  speaker;
            QString  target;
            QString  voicetype;
            QString  faction;
            QString  conditions;
            QString  conditions_tooltip;
            bool     links_to_any_topics = false;
            bool     has_end_fragment    = false;
            bool     has_own_prompt      = false;
            bool     uses_shared_info    = false;
            float    hours_until_reset   = 0;
            uint32_t flags = 0;
         } cached;

         void recache_conditions_from_stub(
            dovah::form_stub& owning_quest
         );
         void recache_responses_from_stub();
         void recache_from_stub(
            dovah::form_stub& owning_quest
         );
      };

      struct Topic {
         ~Topic();

         Branch* parent = nullptr;

         dovah::form_stub*  stub = nullptr;
         std::vector<Info*> infos;
         struct {
            QString  editor_id;
            QString  display_text;
            uint8_t  priority = 0;
            uint32_t subtype  = 0; // signature

            dovah::dialogue::category category = dovah::dialogue::category::topic;
         } cached;

         constexpr size_t index_of_info(const dovah::form_stub&) const;
         void recache_from_stub();
      };

      struct Branch {
         enum class Type {
            Normal,
            Blocking,
            TopLevel,
         };

         ~Branch();

         dovah::form_stub*   stub = nullptr;
         std::vector<Topic*> topics;
         struct {
            QString editor_id;
            Type    type      = Type::Normal;
            bool    exclusive = false;
         } cached;
         Topic* starting_topic = nullptr;

         void recache_from_stub();
      };

   public:
      struct {
         std::vector<Branch*> branches;
         std::vector<Topic*>  branchless_topics; // generally anything that isn't Player Dialogue
      } _data;
      dovah::form_stub* _quest = nullptr;

      void _clear(bool emit_signals);

      Branch* _item_for_branch_stub(const dovah::form_stub&) const;
      Topic* _item_for_topic_stub(const dovah::form_stub&) const;
      std::pair<Branch*, size_t> _locate_topic(dovah::form_stub&, bool bypass_form_use_info = false) const;

   public:
      dovah::form_stub* get_quest() const;
      void set_quest(dovah::form_stub*);

      const std::vector<const Branch*>& all_branches() const;
      const std::vector<const Topic*>& all_branchless_topics() const;

      const Branch* item_for_branch_stub(const dovah::form_stub&) const;
      const Topic* item_for_topic_stub(const dovah::form_stub&) const;

      #pragma region Helper functions for form structure
         void reorder_info(const Info&, int by);
         void reorder_infos(const Topic&, size_t start, size_t count, int by); // I wish there was a language convention for "The caller can't modify this, so it's const&, but the callee can."
      #pragma endregion

      // When creating a new topic, you'll want to consult these lists and ensure that the 
      // topic subtype you wish to create is actually available (i.e. it's reusalbe or it 
      // isn't already being used at the destination).
      [[nodiscard]] std::vector<const dovah::dialogue::topic_subtype*> get_available_branchless_topic_subtypes(dovah::dialogue::category) const;
      [[nodiscard]] std::vector<const dovah::dialogue::topic_subtype*> get_available_branch_topic_subtypes(const Branch&) const;

   signals:
      void on_cleared();
      void on_filled();

      void on_branch_added(const Branch&);
      void on_branch_edited(const Branch&);
      void on_branch_removed(const Branch&); // emitted after the node is removed from the datastore and just before it's deleted

      // A view of a branch's topics must listen for the topic signals, as well as for `on_cleared` and `on_branch_removed`.
      void on_topic_added(const Topic&);
      void on_topic_edited(const Topic&);
      void on_topic_removed(const Topic&); // emitted after the node is removed from the datastore and just before it's deleted; node.parent still points to the old parent

      // A view of a topic's infos must listen for the info signals, as well as for `on_cleared`, `on_branch_removed`, and `on_topic_removed`.
      void on_info_added(const Info&);
      void on_info_edited(const Info&);
      void on_info_removed(const Info&); // emitted after the node is removed from the datastore and just before it's deleted; node.parent still points to the old parent
      void on_info_reordered(const Info&);

   protected:
      void _sync_info_order_to_form(const Topic&);

      Topic* _make_datastore_item_for_topic(dovah::form_stub& topic);

      void _add_branch_to_datastore(dovah::form_stub& branch); // also adds contained topics; does not emit signals
      void _add_branchless_topic_to_datastore(dovah::form_stub& topic); // also adds contained infos; does not emit signals
      void _add_topic_to_datastore(Branch&, dovah::form_stub& topic); // also adds contained infos; does not emit signals
      void _add_info_to_datastore(Topic&, dovah::form_stub& info); // does not emit signals

      void _remove_branch_from_datastore(size_t i);
      void _remove_branchless_topic_from_datastore(size_t i);
      void _remove_topic_from_datastore(Branch&, size_t i);
      void _remove_info_from_datastore(Topic&, size_t i);

      //
      // Handlers below. This datastore is not an interface for modifying the branches, 
      // topics, or infos; rather, outside code should load those forms and modify them 
      // as normal. This datastore will then detect those modifications.
      //

      void _handle_branch_edited(dovah::form_stub&);
      void _handle_topic_edited(dovah::form_stub&);
      void _handle_info_edited(dovah::form_stub&);

      void _handle_branch_created(dovah::form_stub&);
      void _handle_topic_created(dovah::form_stub&);
      void _handle_info_created(dovah::form_stub&);

      void _handle_branch_removed(dovah::form_stub&);
      void _handle_topic_removed(dovah::form_stub&);
      void _handle_info_deleted(dovah::form_stub&, bool just_being_flagged);

      // CK behavior:
      //  - Infos that are flagged as deleted still display in the listing.
      //  - Topics that are flagged as deleted are hidden entirely.
      //  - Branches that are flagged as deleted are probably also hidden entirely.
      void _handle_form_deleted(dovah::form_stub&, bool just_being_flagged);
};

#include "./QuestAllDialogueDatastore.inl"