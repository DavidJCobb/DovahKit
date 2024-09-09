#include "./QuestAllDialogueModel.h"
#include "dovah/form_stubs/helpers/for_each_child_form.h"
#include "dovah/form_stubs/helpers/for_each_dialogue_branch_topic.h"
#include "dovah/form_stubs/helpers/for_each_quest_dialogue_branch.h"
#include "dovah/form_stubs/helpers/for_each_quest_topic.h"
#include "dovah/form_stubs/helpers/get_dialogue_branch_quest.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_branch.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_quest.h"
#include "dovah/forms/DialogueBranch.h"
#include "dovah/forms/Topic.h"
#include "dovah/forms/TopicInfo.h"
#include "editor/core.h"

#pragma region Info
   void QuestAllDialogueModel::Info::recache_from_stub();
#pragma endregion

#pragma region Topic
   QuestAllDialogueModel::Topic::~Topic() {
      for (auto* item : this->infos)
         delete item;
      this->infos.clear();
   }
   void QuestAllDialogueModel::Topic::recache_from_stub();
#pragma endregion

#pragma region Branch
   QuestAllDialogueModel::Branch::~Branch() {
      for (auto* item : this->topics)
         delete item;
      this->topics.clear();
   }
#pragma endregion

#pragma region QuestAllDialogueModel
   QuestAllDialogueModel::QuestAllDialogueModel(QObject* parent) : QAbstractItemModel(parent) {
      //
      // TODO: form modify/delete/create signals
      //
      auto& editor = DovahKitCore::get();
   }
   QuestAllDialogueModel::~QuestAllDialogueModel() {
      this->_clear(false);
   }

   void QuestAllDialogueModel::_clear(bool emit_signals) {
      if (emit_signals)
         this->beginResetModel();
      {
         auto& list = this->_data.branches;
         for (auto* item : list)
            delete item;
         list.clear();
      }
      {
         auto& list = this->_data.branchless_topics;
         for (auto* item : list)
            delete item;
         list.clear();
      }
      if (emit_signals)
         this->endResetModel();
   }

   QModelIndex QuestAllDialogueModel::_qmi_for_form(dovah::form_stub& stub, size_t row, size_t col) const {
      return this->createIndex(row, col, &stub);
   }
   dovah::form_stub* QuestAllDialogueModel::_form_from_qmi(const QModelIndex& qmi) const {
      if (qmi == this->branchlessTopicRoot())
         return nullptr;
      return (dovah::form_stub*)qmi.internalPointer();
   }

   QuestAllDialogueModel::Branch* QuestAllDialogueModel::_branch_for_stub(const dovah::form_stub& stub) const {
      if (this->_quest == nullptr)
         return nullptr;
      if (dovah::form_stub_helpers::get_dialogue_branch_quest(&stub) != this->_quest)
         return nullptr;
      for (auto* branch : this->_data.branches)
         if (branch->stub == &stub)
            return branch;
      return nullptr;
   }
   QuestAllDialogueModel::Topic* QuestAllDialogueModel::_topic_for_stub(const dovah::form_stub& stub) const {
      if (this->_quest == nullptr)
         return nullptr;
      if (dovah::form_stub_helpers::get_dialogue_topic_quest(&stub) != this->_quest)
         return nullptr;

      auto* branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(&stub);
      if (branch_stub) {
         for (auto* branch : this->_data.branches) {
            if (branch->stub != branch_stub)
               continue;
            for (auto* topic : branch->topics)
               if (topic->stub == &stub)
                  return topic;
            return nullptr;
         }
      } else {
         for (auto* topic : this->_data.branchless_topics)
            if (topic->stub == &stub)
               return topic;
      }
      return nullptr;
   }
   QuestAllDialogueModel::Info* QuestAllDialogueModel::_info_for_stub(const dovah::form_stub& stub) const {
      if (this->_quest == nullptr)
         return nullptr;
      if (stub.form_type != dovah::form_type::topic_info)
         return nullptr;

      auto* topic_stub = stub.get_parent_form();
      if (!topic_stub)
         return nullptr;
      auto* topic = this->_topic_for_stub(*topic_stub);
      if (!topic)
         return nullptr;
      for (auto* info : topic->infos)
         if (info->stub == &stub)
            return info;
      return nullptr;
   }
   
   #pragma region QAbstractItemModel overrides
      #pragma region Hierarchy
         /*virtual*/ QModelIndex QuestAllDialogueModel::index(int row, int column, const QModelIndex& parent) const /*override*/;
         /*virtual*/ QModelIndex QuestAllDialogueModel::parent(const QModelIndex& index) const /*override*/;
         /*virtual*/ QModelIndex QuestAllDialogueModel::sibling(int row, int column, const QModelIndex& index) const /*override*/;
         /*virtual*/ int QuestAllDialogueModel::rowCount(const QModelIndex& parent) const /*override*/ {
            if (!parent.isValid())
               return this->_data.branches.size();
            if (parent == this->branchlessTopicRoot())
               return this->_data.branchless_topics.size();

            auto* form = _form_from_qmi(parent);
            if (!form)
               return 0;
            switch (form->form_type) {
               case dovah::form_type::dialogue_branch:
                  if (auto* data = this->_branch_for_stub(*form))
                     return data->topics.size();
                  break;
               case dovah::form_type::topic:
                  if (auto* data = this->_topic_for_stub(*form))
                     return data->infos.size();
                  break;
            }
            return 0;
         }
         /*virtual*/ int QuestAllDialogueModel::columnCount(const QModelIndex& parent) const /*override*/ {
            if (!parent.isValid())
               return BranchColumnCount;
            if (parent == this->branchlessTopicRoot())
               return TopicColumnCount;

            auto* form = _form_from_qmi(parent);
            if (!form)
               return 0;
            switch (form->form_type) {
               case dovah::form_type::dialogue_branch:
                  return TopicColumnCount;
               case dovah::form_type::topic:
                  return InfoColumnCount;
            }
            return 0;
         }
      #pragma endregion
      #pragma region Node data
         /*virtual*/ QVariant QuestAllDialogueModel::data(const QModelIndex& index, int role) const /*override*/;
         /*virtual*/ Qt::ItemFlags QuestAllDialogueModel::flags(const QModelIndex& index) const /*override*/;
      #pragma endregion
      /*virtual*/ QVariant QuestAllDialogueModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case BranchColumn::EditorID:
               return tr("Editor ID");
            case BranchColumn::FormID:
               return tr("Form ID");
            case BranchColumn::Flags:
               return tr("Flags");
         }
         return {};
      }
   #pragma endregion

   void QuestAllDialogueModel::setQuest(dovah::form_stub* stub) {
      if (stub == this->_quest)
         return;
      this->beginResetModel();
      this->_clear(false);

      auto _get_infos_for_topic = [this](Topic& topic) {
         dovah::form_stub_helpers::for_each_child_form(topic.stub, [this, &topic](dovah::form_stub* child) {
            if (child->form_type != dovah::form_type::topic_info)
               return;

            auto* info = topic.infos.emplace_back() = new Info;
            info->stub = child;
            info->recache_from_stub();
         });
      };

      dovah::form_stub_helpers::for_each_quest_dialogue_branch(stub, [this, &_get_infos_for_topic](dovah::form_stub* branch_stub) {
         auto* branch = this->_data.branches.emplace_back() = new Branch;
         branch->stub   = branch_stub;
         branch->loaded = branch_stub->load().ptr_cast<dovah::loaded_forms::DialogueBranch>();

         dovah::form_stub_helpers::for_each_dialogue_branch_topic(branch_stub, [this, branch, &_get_infos_for_topic](dovah::form_stub* topic_stub) {
            auto* topic = branch->topics.emplace_back() = new Topic;
            topic->stub = topic_stub;
            topic->recache_from_stub();
            _get_infos_for_topic(*topic);
         });
      });
      dovah::form_stub_helpers::for_each_quest_topic(stub, [this, &_get_infos_for_topic](dovah::form_stub* topic_stub) {
         auto* branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(topic_stub);
         if (branch_stub)
            return;

         auto* topic = this->_data.branchless_topics.emplace_back() = new Topic;
         topic->stub = topic_stub;
         topic->recache_from_stub();
         _get_infos_for_topic(*topic);
      });

      this->endResetModel();
   }

   QModelIndex QuestAllDialogueModel::index(const dovah::form_stub& stub) const {
      if (!this->_quest)
         return {};

      dovah::form_stub* stub_quest = nullptr;
      switch (stub.form_type) {
         case dovah::form_type::dialogue_branch:
            stub_quest = dovah::form_stub_helpers::get_dialogue_branch_quest(&stub);
            break;
         case dovah::form_type::topic:
            stub_quest = dovah::form_stub_helpers::get_dialogue_topic_quest(&stub);
            break;
         case dovah::form_type::topic_info:
            {
               auto* topic = stub.get_parent_form();
               if (!topic || topic->form_type != dovah::form_type::topic)
                  return {};
               stub_quest = dovah::form_stub_helpers::get_dialogue_topic_quest(topic);
            }
            break;
         default:
            return {};
      }
      if (stub_quest != this->_quest)
         return {};

      return this->_qmi_for_form(stub);
   }

   void QuestAllDialogueModel::_on_branch_edited(dovah::form_stub& stub) {
      static_assert(false, "TODO: Account for the branch being moved to another quest.");
      static_assert(false, "TODO: If it wasn't moved to another quest, emit data-changed on the branch as appropriate.");
   }
   void QuestAllDialogueModel::_on_topic_edited(dovah::form_stub& stub) {
      static_assert(false, "TODO: Account for the topic being moved to another branch.");
      static_assert(false, "TODO: Recache the topic data and emit data-changed.");
   }
   void QuestAllDialogueModel::_on_info_edited(dovah::form_stub& stub) {
      static_assert(false, "TODO: Account for the info being moved to another topic.");
      static_assert(false, "TODO: Recache the info data and emit data-changed.");
   }

   void QuestAllDialogueModel::_on_branch_created(dovah::form_stub& stub);
   void QuestAllDialogueModel::_on_topic_created(dovah::form_stub& stub);
   void QuestAllDialogueModel::_on_info_created(dovah::form_stub& stub);

   // CK behavior:
   //  - Infos that are flagged as deleted still display in the listing.
   //  - Topics that are flagged as deleted are hidden entirely.
   //  - Branches that are flagged as deleted are probably also hidden entirely.
   void QuestAllDialogueModel::_on_form_deleted(dovah::form_stub& stub, bool just_being_flagged);
#pragma endregion

#pragma region QuestBranchesModel
   /*virtual*/ QVariant QuestBranchesModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      switch (section) {
         case Column::EditorID:
            return tr("Editor ID");
         case Column::FormID:
            return tr("Form ID");
         case Column::Flags:
            return tr("Flags");
      }
      return {};
   }

   /*virtual*/ bool QuestBranchesModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const /*override*/;
   /*virtual*/ bool QuestBranchesModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const /*override*/;
#pragma endregion

#pragma region QuestBranchTopicsModel
   /*virtual*/ QVariant QuestBranchTopicsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      switch (section) {
         case Column::EditorID:
            return tr("Editor ID");
         case Column::FormID:
            return tr("Form ID");
         case Column::IsBranchStartingTopic:
            return tr("Starting");
         case Column::Priority:
            return tr("Priority");
         case Column::DisplayText:
            return tr("Display Text");
      }
      return {};
   }

   /*virtual*/ bool QuestBranchTopicsModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const /*override*/;
#pragma endregion

#pragma region QuestBranchlessTopicsModel
   /*virtual*/ QVariant QuestBranchlessTopicsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      switch (section) {
         case Column::EditorID:
            return tr("Editor ID");
         case Column::FormID:
            return tr("Form ID");
         case Column::IsBranchStartingTopic:
            return tr("Starting");
         case Column::Priority:
            return tr("Priority");
         case Column::DisplayText:
            return tr("Display Text");
      }
      return {};
   }

   /*virtual*/ bool QuestBranchlessTopicsModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const /*override*/;
   /*virtual*/ bool QuestBranchlessTopicsModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const /*override*/;
#pragma endregion

#pragma region QuestTopicInfoModel
   /*virtual*/ QVariant QuestTopicInfoModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      switch (section) {
         case Column::InfoText:
            return tr("Text");
         case Column::EditorID:
            return tr("Editor ID");
         case Column::FormID:
            return tr("Form ID");
         case Column::Flags:
            return tr("Flags");
         case Column::ResponseCount:
            return tr("# Responses");
         case Column::Speaker:
            return tr("Speaker");
         case Column::Target:
            return tr("Target");
         case Column::IsVoiceType:
            return tr("Voicetype");
         case Column::InFaction:
            return tr("Faction");
         case Column::Conditions:
            return tr("Conditions");
         case Column::HasResultScript:
            return tr("Has Result Script");
      }
      return {};
   }
#pragma endregion