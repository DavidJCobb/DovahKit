#include "./QuestAllDialogueModel.h"
#include <cassert>
#include "dovah/data/conditions/all_function_info.h"
#include "dovah/data/dialogue/topic_subtype.h"
#include "dovah/form_stubs/helpers/for_each_child_form.h"
#include "dovah/form_stubs/helpers/for_each_dialogue_branch_topic.h"
#include "dovah/form_stubs/helpers/for_each_quest_dialogue_branch.h"
#include "dovah/form_stubs/helpers/for_each_quest_topic.h"
#include "dovah/form_stubs/helpers/get_dialogue_branch_quest.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_branch.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_quest.h"
#include "dovah/forms/components/papyrus/fragment_data/topic_info_fragment_data.h"
#include "dovah/forms/DialogueBranch.h"
#include "dovah/forms/Topic.h"
#include "dovah/forms/TopicInfo.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_addenda.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/helpers/stringify_conditions.h"
#include "editor/core.h"

#pragma region Info
   void QuestAllDialogueModel::Info::recache_from_stub(dovah::form_stub& owning_quest) {
      using loaded_form_type = dovah::loaded_forms::TopicInfo;

      dovah::loaded_form_ptr<loaded_form_type> loaded;
      if (auto* stub = this->stub)
         loaded = stub->load().ptr_cast<loaded_form_type>();

      if (!loaded) {
         this->cached = {};
         if (this->stub)
            this->cached.editor_id = QString::fromStdString(this->stub->editorID);
         return;
      }

      this->cached.editor_id = QString::fromStdString(this->stub->editorID);
      this->cached.response_count = loaded->responses.size();
      {
         const QString delim = tr(" | ");

         auto& editor = DovahKitCore::get();
         auto& dst    = this->cached.responses;
         for (size_t i = 0; i < loaded->responses.size(); ++i) {
            auto str = editor.convert_localized_string(loaded->responses[i].text);
            if (i > 0) {
               str += delim;
            }
            str += dst;
         }
      }
      this->cached.flags = loaded->info_flags;
      this->cached.uses_shared_info    = loaded->use_shared_info != nullptr;
      this->cached.links_to_any_topics = !loaded->link_to.normal.empty() || !loaded->link_to.locked.empty();
      this->cached.hours_until_reset   = loaded->hours_until_reset;
      {
         auto& editor = DovahKitCore::get();
         auto  prompt = editor.convert_localized_string(loaded->override_topic_text);
         if (!prompt.isEmpty())
            this->cached.has_own_prompt = true;
      }
      {
         this->cached.has_end_fragment = false;
         if (auto* fragments = loaded->script_data.fragment_data) {
            assert(fragments->type == dovah::loaded_forms::components::papyrus::fragment_type::info);
            auto* casted = (dovah::loaded_forms::components::papyrus::topic_info_fragment_data*)fragments;
            if (casted->fragments.on_end.has_value())
               this->cached.has_end_fragment = true;
         }
      }
      {
         constexpr const auto or_flag = dovah::loaded_forms::components::condition::flag::or_linked;

         dovah::loaded_forms::components::conditions::context context;
         context.quest = &owning_quest;

         size_t end = loaded->conditions.locked.size() + loaded->conditions.normal.size();
         auto&  dst = this->cached.conditions;

         auto _stringify_list = [&context, &dst, end](const dovah::loaded_forms::components::condition_list& list, size_t offset = 0) {
            size_t size = list.size();
            for (size_t i = 0; i < size - 1; ++i) {
               const auto& src = list[i];
               dst += editor_helpers::stringify_condition(src, context);
               dst += ' ';
               dst += editor_helpers::stringify_condition_boolean_operator(src);
               dst += ' ';
            }
            if (size) {
               const auto& src = list[size - 1];
               dst += editor_helpers::stringify_condition(src, context);
               if (size < end) {
                  dst += ' ';
                  dst += editor_helpers::stringify_condition_boolean_operator(src);
                  dst += ' ';
               }
            }
         };
         _stringify_list(loaded->conditions.locked);
         _stringify_list(loaded->conditions.normal, loaded->conditions.locked.size());
      }
      {
         constexpr const auto id_GetIsID        = dovah::conditions::function_id_by_name("GetIsID");
         constexpr const auto id_GetInFaction   = dovah::conditions::function_id_by_name("GetInFaction");
         constexpr const auto id_GetIsVoicetype = dovah::conditions::function_id_by_name("GetIsVoicetype");

         struct logical_operator_state {
            bool inside_or  = false;
            bool last_is_or = false;
         };

         struct {
            logical_operator_state speaker;
            logical_operator_state target;
            logical_operator_state faction;
            logical_operator_state voicetype;
         } states;

         auto _process = [this, &states](const dovah::loaded_forms::components::condition_list& list) {
            constexpr const auto or_flag = dovah::loaded_forms::components::condition::flag::or_linked;

            auto _form_arg = [](const dovah::loaded_forms::components::conditions::working_parameter& variant) -> dovah::form_stub* {
               if (!std::holds_alternative<dovah::form_stub*>(variant))
                  return nullptr;
               return std::get<dovah::form_stub*>(variant);
            };

            for (const auto& condition : list) {
               const auto run_on = condition.get_run_on_data();
               const auto func   = condition.get_function_id();

               const dovah::form_stub* param = nullptr;
               logical_operator_state* logic = nullptr;
               QString* dst_p = nullptr;

               bool this_is_or = condition.test_flags(or_flag);

               bool this_is_negated = false;
               {
                  auto cmp = condition.get_comparison();
                  if (std::holds_alternative<float>(cmp.operand)) {
                     float operand = std::get<float>(cmp.operand);
                     switch (cmp.op) {
                        using enum dovah::loaded_forms::components::comparison_operator;
                        case equal:
                           this_is_negated = operand != 1;
                           break;
                        case not_equal:
                           this_is_negated = operand == 1;
                           break;
                        case less:
                           this_is_negated = operand <= 1;
                           break;
                        case less_or_equal:
                           this_is_negated = operand < 1;
                           break;
                        case greater:
                           this_is_negated = operand >= 1;
                           break;
                        case greater_or_equal:
                           this_is_negated = operand > 1;
                           break;
                     }
                  } else {
                     //
                     // Skip comparisons to globals.
                     //
                     continue;
                  }
               }

               switch (func) {
                  case id_GetIsID:
                     if (run_on.type != dovah::conditions::run_on_type::subject && run_on.type != dovah::conditions::run_on_type::target) {
                        break;
                     }
                     {
                        param = _form_arg(condition.get_parameter(0));
                        if (!param || param->form_type != dovah::form_type::actor_base)
                           break;

                        bool is_subject = run_on.type == dovah::conditions::run_on_type::subject;

                        logic = &(is_subject ? states.speaker : states.target);
                        dst_p = &(is_subject ? this->cached.speaker : this->cached.target);
                     }
                     break;
                  case id_GetInFaction:
                     if (run_on.type != dovah::conditions::run_on_type::subject)
                        break;
                     {
                        param = _form_arg(condition.get_parameter(0));
                        if (!param || param->form_type != dovah::form_type::faction)
                           break;

                        logic = &states.faction;
                        dst_p = &this->cached.faction;
                     }
                     break;
                  case id_GetIsVoicetype:
                     if (run_on.type != dovah::conditions::run_on_type::subject)
                        break;
                     {
                        param = _form_arg(condition.get_parameter(0));
                        if (!param)
                           break;
                        if (param->form_type != dovah::form_type::voicetype && param->form_type != dovah::form_type::formlist)
                           break;

                        logic = &states.voicetype;
                        dst_p = &this->cached.voicetype;
                     }
                     break;
               }

               if (dst_p) {
                  assert(logic != nullptr);
                  assert(param != nullptr);
                  auto& dst = *dst_p;
                  if (!dst.isEmpty()) {
                     if (logic->last_is_or) {
                        dst += " | ";
                     } else {
                        if (logic->inside_or) {
                           dst += ")";
                        }
                        dst += " & ";
                        logic->inside_or = this_is_or;
                        if (this_is_or)
                           dst += "(";
                     }
                  }
                  if (this_is_negated) {
                     dst += "NOT ";
                  }
                  dst += QString::fromStdString(param->editorID);
               }
            }
         };

         _process(loaded->conditions.locked);
         _process(loaded->conditions.normal);

         auto _finalize = [](const logical_operator_state& state, QString& dst) {
            if (state.inside_or)
               dst += ")";
         };
         _finalize(states.speaker,   this->cached.speaker);
         _finalize(states.target,    this->cached.target);
         _finalize(states.faction,   this->cached.faction);
         _finalize(states.voicetype, this->cached.voicetype);
      }
   }
#pragma endregion

#pragma region Topic
   QuestAllDialogueModel::Topic::~Topic() {
      for (auto* item : this->infos)
         delete item;
      this->infos.clear();
   }
   void QuestAllDialogueModel::Topic::recache_from_stub() {
      using loaded_form_type = dovah::loaded_forms::Topic;

      dovah::loaded_form_ptr<loaded_form_type> loaded;
      if (auto* stub = this->stub)
         loaded = stub->load().ptr_cast<loaded_form_type>();

      if (!loaded) {
         this->cached = {};
         if (this->stub)
            this->cached.editor_id = QString::fromStdString(this->stub->editorID);
         return;
      }
      this->cached.editor_id = QString::fromStdString(this->stub->editorID);
      {
         auto& editor = DovahKitCore::get();
         this->cached.display_text = editor.convert_localized_string(loaded->text);
      }
      this->cached.priority = loaded->priority;
      this->cached.subtype  = loaded->subtype;
      {
         const auto* info = dovah::dialogue::topic_subtype_by_signature(this->cached.subtype);
         if (info)
            this->cached.category = info->category;
      }
   }
#pragma endregion

#pragma region Branch
   QuestAllDialogueModel::Branch::~Branch() {
      for (auto* item : this->topics)
         delete item;
      this->topics.clear();
      this->starting_topic = nullptr;
   }
   void QuestAllDialogueModel::Branch::recache_from_stub() {
      using loaded_form_type = dovah::loaded_forms::DialogueBranch;

      dovah::loaded_form_ptr<loaded_form_type> loaded;
      if (auto* stub = this->stub)
         loaded = stub->load().ptr_cast<loaded_form_type>();

      if (!loaded) {
         this->cached = {};
         if (this->stub)
            this->cached.editor_id = QString::fromStdString(this->stub->editorID);
         return;
      }
      this->cached.editor_id = QString::fromStdString(this->stub->editorID);

      this->cached.exclusive = loaded->branch_flags & loaded_form_type::branch_flag::exclusive;
      if (loaded->branch_flags & loaded_form_type::branch_flag::blocking) {
         this->cached.type = Type::Blocking;
      } else if (loaded->branch_flags & loaded_form_type::branch_flag::top_level) {
         this->cached.type = Type::TopLevel;
      } else {
         this->cached.type = Type::Normal;
      }

      this->starting_topic = nullptr;
      for (auto* topic : this->topics) {
         if (topic->stub == loaded->starting_topic.get_form_stub()) {
            this->starting_topic = topic;
            break;
         }
      }
      if (!this->starting_topic && !this->topics.empty())
         this->starting_topic = this->topics[0];
   }
#pragma endregion

#pragma region QuestAllDialogueModel
   QuestAllDialogueModel::QuestAllDialogueModel(QObject* parent) : QAbstractItemModel(parent) {
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::formCreated, this, [this](dovah::form_stub* stub) {
         switch (stub->form_type) {
            case dovah::form_type::dialogue_branch:
               this->_on_branch_created(*stub);
               break;
            case dovah::form_type::topic:
               this->_on_topic_created(*stub);
               break;
            case dovah::form_type::topic_info:
               this->_on_info_created(*stub);
               break;
         }
      });
      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
         switch (stub->form_type) {
            case dovah::form_type::dialogue_branch:
               this->_on_branch_edited(*stub);
               break;
            case dovah::form_type::topic:
               this->_on_topic_edited(*stub);
               break;
            case dovah::form_type::topic_info:
               this->_on_info_edited(*stub);
               break;
         }
      });
      QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
         switch (stub->form_type) {
            case dovah::form_type::quest:
               if (stub == this->_quest) {
                  this->_quest = nullptr;
                  this->_clear(false);
                  return;
               }
               break;
            case dovah::form_type::dialogue_branch:
            case dovah::form_type::topic:
            case dovah::form_type::topic_info:
               this->_on_form_deleted(*stub, just_being_flagged);
               break;
         }
      });
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         this->_clear(false);
         this->_quest = nullptr;
      });
   }
   QuestAllDialogueModel::~QuestAllDialogueModel() {
      this->_clear(false);
      this->_quest = nullptr;
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
         /*virtual*/ QModelIndex QuestAllDialogueModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
            if (row < 0 || column < 0)
               return {};
            if (parent == this->branchlessTopicRoot()) {
               if (column >= TopicColumnCount)
                  return {};
               auto& list = this->_data.branchless_topics;
               if (row >= list.size())
                  return {};
               assert(list[row]->stub != nullptr);
               return _qmi_for_form(*list[row]->stub, row, column);
            } else if (!parent.isValid()) {
               if (column >= BranchColumnCount)
                  return {};
               auto& list = this->_data.branches;
               if (row >= list.size())
                  return {};
               assert(list[row]->stub != nullptr);
               return _qmi_for_form(*list[row]->stub, row, column);
            } else {
               auto* parent_stub = _form_from_qmi(parent);
               if (!parent_stub)
                  return {};
               switch (parent_stub->form_type) {
                  case dovah::form_type::dialogue_branch:
                     //
                     // Get child Topic.
                     //
                     if (column >= TopicColumnCount)
                        return {};
                     if (auto* parent_item = this->_branch_for_stub(*parent_stub)) {
                        auto& list = parent_item->topics;
                        if (row < list.size()) {
                           assert(list[row]->stub != nullptr);
                           return _qmi_for_form(*list[row]->stub, row, column);
                        }
                     }
                     break;
                  case dovah::form_type::topic:
                     //
                     // Get child Info.
                     //
                     if (column >= InfoColumnCount)
                        return {};
                     if (auto* parent_item = this->_topic_for_stub(*parent_stub)) {
                        auto& list = parent_item->infos;
                        if (row < list.size()) {
                           assert(list[row]->stub != nullptr);
                           return _qmi_for_form(*list[row]->stub, row, column);
                        }
                     }
                     break;
                  case dovah::form_type::topic_info:
                     //
                     // Infos cannot have children.
                     //
                     break;
               }
               return {};
            }
         }
         /*virtual*/ QModelIndex QuestAllDialogueModel::parent(const QModelIndex& qmi) const /*override*/ {
            if (!qmi.isValid() || qmi == this->branchlessTopicRoot())
               return {};
            auto* stub = _form_from_qmi(qmi);
            if (!stub)
               return {};
            switch (stub->form_type) {
               case dovah::form_type::dialogue_branch:
                  return {};
               case dovah::form_type::topic:
                  if (auto* parent_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(stub)) {
                     return _qmi_for_form(*parent_stub);
                  }
                  return this->branchlessTopicRoot();
               case dovah::form_type::topic_info:
                  if (auto* parent_stub = stub->get_parent_form(); parent_stub && parent_stub->form_type == dovah::form_type::topic) {
                     return _qmi_for_form(*parent_stub);
                  }
                  #if _DEBUG
                     __debugbreak(); // Orphaned info?
                  #endif
                  break;
            }
            return {};
         }
         /*virtual*/ QModelIndex QuestAllDialogueModel::sibling(int row, int column, const QModelIndex& qmi) const /*override*/ {
            if (row < 0 || column < 0)
               return {};
            if (!qmi.isValid() || qmi == this->branchlessTopicRoot())
               return {};
            auto* stub = _form_from_qmi(qmi);
            if (!stub)
               return {};
            switch (stub->form_type) {
               case dovah::form_type::dialogue_branch:
                  {
                     auto& list = this->_data.branches;
                     if (column >= BranchColumnCount || row >= list.size())
                        return {};
                     return _qmi_for_form(*list[row]->stub, row, column);
                  }
                  break;
               case dovah::form_type::topic:
                  {
                     if (column >= TopicColumnCount)
                        return {};
                     auto* branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(stub);
                     if (branch_stub) {
                        auto* branch = this->_branch_for_stub(*branch_stub);
                        if (!branch)
                           return {};
                        auto& list = branch->topics;
                        if (row >= list.size())
                           return {};
                        return _qmi_for_form(*list[row]->stub, row, column);
                     } else {
                        auto& list = this->_data.branchless_topics;
                        if (row >= list.size())
                           return {};
                        return _qmi_for_form(*list[row]->stub, row, column);
                     }
                  }
                  break;
               case dovah::form_type::topic_info:
                  {
                     if (column >= InfoColumnCount)
                        return {};
                     auto* topic_stub = stub->get_parent_form();
                     if (!topic_stub || topic_stub->form_type != dovah::form_type::topic)
                        return {};
                     auto* topic = this->_topic_for_stub(*topic_stub);
                     if (!topic)
                        return {};
                     auto& list = topic->infos;
                     if (row >= list.size())
                        return {};
                     return _qmi_for_form(*list[row]->stub, row, column);
                  }
                  break;
            }
            return {};
         }
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
         /*virtual*/ QVariant QuestAllDialogueModel::data(const QModelIndex& index, int role) const /*override*/ {
            if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
               return {};
            auto* stub = _form_from_qmi(index);
            if (!stub)
               return {};
            switch (stub->form_type) {
               case dovah::form_type::dialogue_branch:
                  if (auto* item = _branch_for_stub(*stub)) {
                     switch (index.column()) {
                        case BranchColumn::EditorID:
                           return item->cached.editor_id;
                        case BranchColumn::FormID:
                           return editor_helpers::form_id_to_string(item->stub->formID);
                        case BranchColumn::Flags:
                           {
                              QString flags;
                              switch (item->cached.type) {
                                 case Branch::Type::TopLevel:
                                    flags += "T";
                                    break;
                                 case Branch::Type::Blocking:
                                    flags += "B";
                                    break;
                                 default:
                                    flags += " ";
                              }
                              if (item->cached.exclusive) {
                                 flags += "E";
                              }
                           }
                           break;
                     }
                  }
                  break;
               case dovah::form_type::topic:
                  if (auto* item = _topic_for_stub(*stub)) {
                     switch (index.column()) {
                        case TopicColumn::EditorID:
                           return item->cached.editor_id;
                        case TopicColumn::FormID:
                           return editor_helpers::form_id_to_string(item->stub->formID);
                        case TopicColumn::DisplayText:
                           return item->cached.display_text;
                        case TopicColumn::Priority:
                           return item->cached.priority;
                        case TopicColumn::IsBranchStartingTopic:
                           {
                              auto* branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(stub);
                              if (branch_stub) {
                                 auto* branch_item = _branch_for_stub(*branch_stub);
                                 if (branch_item && branch_item->starting_topic == item)
                                    return tr("<", "branch starting topic indicator");
                              }
                           }
                           return {};
                     }
                  }
                  break;
               case dovah::form_type::topic_info:
                  if (auto* item = _info_for_stub(*stub)) {
                     switch (index.column()) {
                        case InfoColumn::EditorID:
                           return item->cached.editor_id;
                        case InfoColumn::FormID:
                           return editor_helpers::form_id_to_string(item->stub->formID);
                        case InfoColumn::Conditions:
                           return item->cached.conditions;
                        case InfoColumn::Flags:
                           {
                              using flag = dovah::loaded_forms::TopicInfo::info_flag;

                              QString out;

                              if (item->cached.links_to_any_topics)
                                 out += tr("C", "info flag");

                              if (item->cached.flags & flag::random_end)
                                 out += tr("E", "info flag");
                              else if (item->cached.flags & flag::random)
                                 out += tr("R", "info flag");

                              if (item->cached.flags & flag::goodbye)
                                 out += tr("G", "info flag");
                              if (item->cached.hours_until_reset)
                                 out += tr("O", "info flag");
                              if (item->cached.has_own_prompt)
                                 out += tr("P", "info flag");
                              if (item->cached.flags & flag::say_once)
                                 out += tr("S", "info flag");
                              if (item->cached.flags & flag::walk_away_invisible_in_menu)
                                 out += tr("V", "info flag");
                              if (item->cached.flags & flag::walk_away)
                                 out += tr("W", "info flag");

                              return out;
                           }
                           break;
                        case InfoColumn::HasResultScript:
                           if (item->cached.has_end_fragment)
                              return tr("Y", "info has end fragment");
                           return {};
                        case InfoColumn::InFaction:
                           return item->cached.faction;
                        case InfoColumn::IsVoiceType:
                           return item->cached.voicetype;
                        case InfoColumn::Speaker:
                           return item->cached.speaker;
                        case InfoColumn::Target:
                           return item->cached.target;
                        case InfoColumn::InfoText:
                           return item->cached.responses;
                        case InfoColumn::ResponseCount:
                           return item->cached.response_count;
                     }
                  }
                  break;
            }
            return {};
         }
         /*virtual*/ Qt::ItemFlags QuestAllDialogueModel::flags(const QModelIndex& index) const /*override*/ {
            auto  flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
            auto* stub  = _form_from_qmi(index);
            if (stub && stub->form_type == dovah::form_type::topic_info) {
               flags |= Qt::ItemNeverHasChildren;
            }
            return flags;
         }
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
         auto _handle_info = [this, &topic](dovah::form_stub* child) {
            if (child->form_type != dovah::form_type::topic_info)
               return;

            auto* info = topic.infos.emplace_back() = new Info;
            info->stub = child;
            info->recache_from_stub(*this->_quest);
         };

         if (auto* addenda = topic.stub->addenda) {
            for (auto* child : addenda->ordered_children) {
               _handle_info(child);
            }
            return;
         }
         dovah::form_stub_helpers::for_each_child_form(topic.stub, _handle_info);
      };

      dovah::form_stub_helpers::for_each_quest_dialogue_branch(stub, [this, &_get_infos_for_topic](dovah::form_stub* branch_stub) {
         auto* branch = this->_data.branches.emplace_back() = new Branch;
         branch->stub   = branch_stub;

         dovah::form_stub* starting_topic = nullptr;

         auto loaded = branch_stub->load().ptr_cast<dovah::loaded_forms::DialogueBranch>();
         if (loaded) {
            using branch_flag = dovah::loaded_forms::DialogueBranch::branch_flag;

            branch->cached.exclusive = loaded->branch_flags & branch_flag::exclusive;
            if (loaded->branch_flags & branch_flag::top_level) {
               branch->cached.type = Branch::Type::TopLevel;
            } else if (loaded->branch_flags & branch_flag::blocking) {
               branch->cached.type = Branch::Type::Blocking;
            }
         }

         dovah::form_stub_helpers::for_each_dialogue_branch_topic(branch_stub, [this, branch, starting_topic, &_get_infos_for_topic](dovah::form_stub* topic_stub) {
            auto* topic = branch->topics.emplace_back() = new Topic;
            topic->stub = topic_stub;
            topic->recache_from_stub();
            _get_infos_for_topic(*topic);

            if (topic_stub == starting_topic) {
               branch->starting_topic = topic;
            }
         });
         if (!branch->starting_topic && !branch->topics.empty()) {
            branch->starting_topic = branch->topics[0];
         }
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
      if (dovah::form_stub_helpers::get_dialogue_topic_quest(&stub) != this->_quest)
         return;

      dovah::form_stub* prior_branch_stub = nullptr;
      dovah::form_stub* after_branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(&stub);
      if (after_branch_stub) {
         Branch* prior_branch_item = nullptr;
         Branch* after_branch_item = nullptr;
         bool    did_data_changed  = false;

         auto& b_list = this->_data.branches;
         for (size_t i = 0; i < b_list.size(); ++i) {
            auto* branch = b_list[i];
            if (branch->stub == after_branch_stub) {
               after_branch_item = branch;
            }
            if (!did_data_changed) {
               auto& t_list = branch->topics;
               for (size_t j = 0; j < t_list.size(); ++j) {
                  auto* topic = t_list[j];
                  if (topic->stub == &stub) {
                     topic->recache_from_stub();

                     auto tl = _qmi_for_form(stub, 0);
                     auto br = tl.siblingAtColumn(TopicColumnCount - 1);
                     emit dataChanged(tl, br);
                     did_data_changed = true;

                     prior_branch_item = branch;
                     prior_branch_stub = branch->stub;
                     if (prior_branch_stub == after_branch_stub)
                        return;
                  }
               }
            }
         }
         if (prior_branch_item != after_branch_item) {
            static_assert(false, "TODO");
         }
      }
      //
      // Topic is not currently in a branch.
      //
      static_assert(false, "TODO: Recache the topic data and emit data-changed.");
   }
   void QuestAllDialogueModel::_on_info_edited(dovah::form_stub& stub) {
      static_assert(false, "TODO: Account for the info being moved to another topic.");
      static_assert(false, "TODO: Recache the info data and emit data-changed.");
   }

   void QuestAllDialogueModel::_on_branch_created(dovah::form_stub& stub);
   void QuestAllDialogueModel::_on_topic_created(dovah::form_stub& stub);
   void QuestAllDialogueModel::_on_info_created(dovah::form_stub& stub);

   void QuestAllDialogueModel::_on_branch_removed(dovah::form_stub& stub) {
      if (dovah::form_stub_helpers::get_dialogue_branch_quest(&stub) != this->_quest)
         return;
      auto& list = this->_data.branches;
      for (size_t i = 0; i < list.size(); ++i) {
         auto* branch = list[i];
         if (branch->stub != &stub)
            continue;
         this->beginRemoveRows({}, i, i);
         list.erase(list.begin() + i);
         delete branch;
         this->endRemoveRows();
         break;
      }
   }
   void QuestAllDialogueModel::_on_topic_removed(dovah::form_stub& stub) {
      if (dovah::form_stub_helpers::get_dialogue_topic_quest(&stub) != this->_quest)
         return;
      auto* branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(&stub);
      if (branch_stub) {
         auto& b_list = this->_data.branches;
         for (size_t i = 0; i < b_list.size(); ++i) {
            auto* branch = b_list[i];
            if (branch->stub != branch_stub)
               continue;
            auto  branch_qmi = _qmi_for_form(*branch_stub, i);
            auto& t_list     = branch->topics;
            for (size_t j = 0; j < t_list.size(); ++j) {
               auto* topic = t_list[j];
               if (topic->stub == &stub) {
                  this->beginRemoveRows(branch_qmi, j, j);
                  t_list.erase(t_list.begin() + i);
                  delete branch;
                  this->endRemoveRows();
                  return;
               }
            }
            break;
         }
      } else {
         auto& list = this->_data.branchless_topics;
         for (size_t i = 0; i < list.size(); ++i) {
            auto* topic = list[i];
            if (topic->stub == &stub) {
               this->beginRemoveRows(this->branchlessTopicRoot(), i, i);
               list.erase(list.begin() + i);
               delete topic;
               this->endRemoveRows();
               return;
            }
         }
      }
   }

   // CK behavior:
   //  - Infos that are flagged as deleted still display in the listing.
   //  - Topics that are flagged as deleted are hidden entirely.
   //  - Branches that are flagged as deleted are probably also hidden entirely.
   void QuestAllDialogueModel::_on_form_deleted(dovah::form_stub& stub, bool just_being_flagged) {
      if (!this->_quest)
         return;
      switch (stub.form_type) {
         case dovah::form_type::dialogue_branch:
            this->_on_branch_removed(stub);
            break;
         case dovah::form_type::topic:
            this->_on_topic_removed(stub);
            break;
         case dovah::form_type::topic_info:
            {
               auto* topic_stub  = stub.get_parent_form();
               if (!topic_stub || topic_stub->form_type != dovah::form_type::topic)
                  return;
               auto* branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(&stub);

               auto _remove_info_from_topic = [this, &stub, topic_stub](Topic* topic) -> bool {
                  if (topic->stub != topic_stub)
                     return false;
                  for (size_t i = 0; i < topic->infos.size(); ++i) {
                     auto& i_list = topic->infos;
                     auto* info   = i_list[i];
                     if (info->stub == &stub) {
                        auto topic_qmi = _qmi_for_form(*topic_stub, i);
                        this->beginRemoveRows(topic_qmi, i, i);
                        i_list.erase(i_list.begin() + i);
                        delete info;
                        this->endRemoveRows();
                        return true;
                     }
                  }
                  return true;
               };

               if (branch_stub) {
                  auto& b_list = this->_data.branches;
                  for (size_t i = 0; i < b_list.size(); ++i) {
                     auto* branch = b_list[i];
                     if (branch->stub != branch_stub)
                        continue;
                     auto& t_list = branch->topics;
                     for (size_t j = 0; j < t_list.size(); ++j)
                        if (_remove_info_from_topic(t_list[j]))
                           break;
                     break;
                  }
               } else {
                  auto& t_list = this->_data.branchless_topics;
                  for (size_t i = 0; i < t_list.size(); ++i)
                     if (_remove_info_from_topic(t_list[i]))
                        break;
               }

            }
            break;
      }
   }
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