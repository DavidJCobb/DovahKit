#include "./QuestAllDialogueDatastore.h"
#include <cassert>
#include "helpers/qt/strings/to_no_op_html.h"
#include "helpers/vectors/move_item_within.h"
#include "helpers/vectors/move_range_within.h"
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
   void QuestAllDialogueDatastore::Info::recache_conditions_from_stub(dovah::form_stub& owning_quest) {
      using condition         = dovah::loaded_forms::components::condition;
      using condition_context = dovah::loaded_forms::components::conditions::context;
      using condition_list    = dovah::loaded_forms::components::condition_list;
      using loaded_form_type  = dovah::loaded_forms::TopicInfo;
      //
      constexpr const auto or_flag = dovah::loaded_forms::components::condition::flag::or_linked;
      //
      constexpr auto _condition_qualifier = [](const condition::comparison_data& cmp) constexpr -> int {
         if (!std::holds_alternative<float>(cmp.operand))
            //
            // Skip comparisons to globals.
            //
            return -1;
         float operand = std::get<float>(cmp.operand);
         switch (cmp.op) {
            using enum dovah::loaded_forms::components::comparison_operator;
            case equal:
               return operand != 1;
            case not_equal:
               return operand == 1;
            case less:
               return operand <= 1;
            case less_or_equal:
               return operand < 1;
            case greater:
               return operand >= 1;
            case greater_or_equal:
               return operand > 1;
         }
      };
      constexpr auto _extract_form_param = [](const dovah::loaded_forms::components::conditions::working_parameter& variant) constexpr -> dovah::form_stub* {
         if (!std::holds_alternative<dovah::form_stub*>(variant))
            return nullptr;
         return std::get<dovah::form_stub*>(variant);
      };

      this->cached.speaker.clear();
      this->cached.target.clear();
      this->cached.faction.clear();
      this->cached.voicetype.clear();

      if (!this->stub)
         return;
      dovah::loaded_form_ptr<loaded_form_type> loaded = stub->load().ptr_cast<loaded_form_type>();
      if (!loaded)
         return;

      struct logical_summary {
         QString& dst;
         bool     prev_was_or  = false;
         bool     inside_or    = false;
         bool     or_confirmed = false; // at least 2 items in the or-grouping

         void append_prev_sign() {
            if (this->dst.isEmpty())
               return;
            if (this->prev_was_or) {
               this->dst += " OR ";
               if (this->inside_or)
                  this->or_confirmed = true;
            } else {
               this->dst += " AND ";
            }
         }
         void enter_or() {
            if (this->inside_or)
               return;
            this->dst += "(";
            this->inside_or = true;
         }
         void close_or() {
            if (this->inside_or) {
               this->dst += ")";
               this->inside_or = false;
            }
            this->prev_was_or = false;
         }

         void finalize() {
            if (this->inside_or) {
               if (this->or_confirmed) {
                  this->close_or();
               } else {
                  auto i = this->dst.lastIndexOf('(');
                  if (i >= 0)
                     this->dst.remove(i, 1);
               }
            }
         }
      };

      struct {
         logical_summary speaker;
         logical_summary target;
         logical_summary faction;
         logical_summary voicetype;
      } states = {
         { this->cached.speaker },
         { this->cached.target },
         { this->cached.faction },
         { this->cached.voicetype },
      };

      auto& list_1 = loaded->conditions.locked;
      auto& list_2 = loaded->conditions.normal;

      dovah::loaded_forms::components::conditions::context context;
      context.quest = &owning_quest;

      auto _process_list = [this, &context, &states](const condition_list& list, bool never_the_end) {
         constexpr const auto id_GetIsID        = dovah::conditions::function_id_by_name("GetIsID");
         constexpr const auto id_GetInFaction   = dovah::conditions::function_id_by_name("GetInFaction");
         constexpr const auto id_GetIsVoiceType = dovah::conditions::function_id_by_name("GetIsVoiceType");

         size_t size = list.size();
         if (!size)
            return;

         auto& dst_cnd = this->cached.conditions;

         for (size_t i = 0; i < size; ++i) {
            bool is_the_end = !never_the_end && i == size - 1;

            const auto& src = list[i];

            {
               QString append = editor_helpers::stringify_condition(src, context);
               if (!is_the_end) {
                  append += ' ';
                  append += editor_helpers::stringify_condition_boolean_operator(src);
                  append += ' ';
               }
               this->cached.conditions += append;
               if (!is_the_end)
                  append.back() = '\n';
               this->cached.conditions_tooltip += append;
            }

            const dovah::form_stub* param = nullptr;
            logical_summary*        logic = nullptr;
            //
            auto& run_on     = src.get_run_on_data();
            bool  is_or      = src.get_flags() & or_flag;
            bool  is_negated = false;
            switch (src.get_function_id()) {
               case id_GetIsID:
                  if (run_on.type == dovah::conditions::run_on_type::subject || run_on.type == dovah::conditions::run_on_type::target) {
                     bool is_subject = run_on.type == dovah::conditions::run_on_type::subject;

                     param = _extract_form_param(src.get_parameter(0));
                     if (!param || param->form_type != dovah::form_type::actor_base)
                        break;
                     logic = &(is_subject ? states.speaker : states.target);
                  }
                  break;
               case id_GetInFaction:
                  if (run_on.type == dovah::conditions::run_on_type::subject) {
                     param = _extract_form_param(src.get_parameter(0));
                     if (!param || param->form_type != dovah::form_type::faction)
                        break;
                     logic = &states.faction;
                  }
                  break;
               case id_GetIsVoiceType:
                  if (run_on.type == dovah::conditions::run_on_type::subject) {
                     param = _extract_form_param(src.get_parameter(0));
                     if (!param)
                        break;
                     if (param->form_type != dovah::form_type::voicetype && param->form_type != dovah::form_type::formlist)
                        break;
                     logic = &states.voicetype;
                  }
                  break;
               default:
                  break;
            }
            QString param_str;
            if (logic && param) {
               switch (_condition_qualifier(src.get_comparison())) {
                  case -1:
                     param_str = "MAYBE ";
                     break;
                  case 0:
                     break;
                  case 1:
                     param_str = "NOT ";
                     break;
               }
               param_str += QString::fromStdString(param->editorID);
            }
            if (logic) {
               logic->append_prev_sign();
            }
            if (is_the_end || !is_or) {
               if (logic) {
                  logic->dst += param_str;
               }
               states.speaker.close_or();
               states.target.close_or();
               states.faction.close_or();
               states.voicetype.close_or();
            } else {
               if (logic) {
                  logic->enter_or();
                  logic->dst += param_str;
               }
            }
            if (logic)
               logic->prev_was_or = is_or;
         }
      };

      _process_list(list_1, !list_2.empty());
      _process_list(list_2, false);
      states.speaker.finalize();
      states.target.finalize();
      states.faction.finalize();
      states.voicetype.finalize();

   }
   void QuestAllDialogueDatastore::Info::recache_responses_from_stub() {
      using loaded_form_type = dovah::loaded_forms::TopicInfo;

      auto& display = this->cached.responses;
      auto& tooltip = this->cached.responses_tooltip;

      this->cached.response_count = 0;
      display.clear();
      tooltip.clear();

      dovah::form_stub* stub = this->stub;
      if (!stub)
         return;
      dovah::loaded_form_ptr<loaded_form_type> loaded;
      {
         loaded = stub->load().ptr_cast<loaded_form_type>();
         if (!loaded)
            return;
         if (loaded->use_shared_info) {
            stub   = loaded->use_shared_info.get_form_stub();
            loaded = stub->load().ptr_cast<loaded_form_type>();
            if (!loaded)
               return;
         }
      }

      this->cached.response_count = loaded->responses.size();

      const QString delim = tr(" | ", "response delimiter (in table cell)");

      auto& editor = DovahKitCore::get();
      for (size_t i = 0; i < loaded->responses.size(); ++i) {
         auto str = editor.convert_localized_string(loaded->responses[i].text);
         if (i > 0) {
            display += delim;
            tooltip += "<hr>";
         }
         display += str;
         tooltip += cobb::qt::strings::to_no_op_html(str, "span"); // So tool-tips for this value can word-wrap.
      }
   }
   void QuestAllDialogueDatastore::Info::recache_from_stub(dovah::form_stub& owning_quest) {
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
      this->recache_responses_from_stub();
      this->cached.flags = loaded->info_flags;
      this->cached.uses_shared_info    = loaded->use_shared_info != nullptr;
      this->cached.links_to_any_topics = !loaded->link_to.normal.empty() || !loaded->link_to.locked.empty();
      this->cached.hours_until_reset   = loaded->get_hours_until_reset();
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
      this->recache_conditions_from_stub(owning_quest); // TEST
   }
#pragma endregion

#pragma region Topic
   QuestAllDialogueDatastore::Topic::~Topic() {
      for (auto* item : this->infos)
         delete item;
      this->infos.clear();
   }

   void QuestAllDialogueDatastore::Topic::recache_from_stub() {
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
         if (info) {
            this->cached.category = info->category;
         } else {
            #if _DEBUG
               __debugbreak(); // Unknown subtype signature?
            #endif
         }
      }
   }
#pragma endregion

#pragma region Branch
   QuestAllDialogueDatastore::Branch::~Branch() {
      for (auto* item : this->topics)
         delete item;
      this->topics.clear();
      this->starting_topic = nullptr;
   }
   void QuestAllDialogueDatastore::Branch::recache_from_stub() {
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

#pragma region QuestAllDialogueDatastore
   QuestAllDialogueDatastore::QuestAllDialogueDatastore(QObject* parent) : QObject(parent) {
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::formCreated, this, [this](dovah::form_stub* stub) {
         switch (stub->form_type) {
            case dovah::form_type::dialogue_branch:
               this->_handle_branch_created(*stub);
               break;
            case dovah::form_type::topic:
               this->_handle_topic_created(*stub);
               break;
            case dovah::form_type::topic_info:
               this->_handle_info_created(*stub);
               break;
         }
      });
      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
         switch (stub->form_type) {
            case dovah::form_type::dialogue_branch:
               this->_handle_branch_edited(*stub);
               break;
            case dovah::form_type::topic:
               this->_handle_topic_edited(*stub);
               break;
            case dovah::form_type::topic_info:
               this->_handle_info_edited(*stub);
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
               this->_handle_form_deleted(*stub, just_being_flagged);
               break;
         }
      });
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         this->_clear(false);
         this->_quest = nullptr;
      });
   }
   QuestAllDialogueDatastore::~QuestAllDialogueDatastore() {
      this->_clear(false);
      this->_quest = nullptr;
   }

   void QuestAllDialogueDatastore::_clear(bool emit_signals) {
      for (auto* item : this->_data.branches)
         delete item;
      this->_data.branches.clear();

      for (auto* item : this->_data.branchless_topics)
         delete item;
      this->_data.branchless_topics.clear();

      if (emit_signals)
         this->on_cleared();
   }

   QuestAllDialogueDatastore::Branch* QuestAllDialogueDatastore::_item_for_branch_stub(const dovah::form_stub& stub) const {
      assert(stub.form_type == dovah::form_type::dialogue_branch);
      for (auto* branch : this->_data.branches)
         if (branch->stub == &stub)
            return branch;
      return nullptr;
   }
   QuestAllDialogueDatastore::Topic* QuestAllDialogueDatastore::_item_for_topic_stub(const dovah::form_stub& stub) const {
      assert(stub.form_type == dovah::form_type::topic);
      auto* branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(&stub);
      if (branch_stub) {
         if (auto* branch = this->_item_for_branch_stub(*branch_stub))
            for (auto* item : branch->topics)
               if (item->stub == &stub)
                  return item;
      } else {
         for (auto* item : this->_data.branchless_topics)
            if (item->stub == &stub)
               return item;
      }
      return nullptr;
   }
   std::pair<QuestAllDialogueDatastore::Branch*, size_t> QuestAllDialogueDatastore::_locate_topic(dovah::form_stub& stub, bool bypass_form_use_info) const {
      constexpr const auto none_result = std::pair<Branch*, size_t>{ nullptr, (size_t)-1 };

      if (bypass_form_use_info) {
         for (auto* branch : this->_data.branches) {
            auto&  list = branch->topics;
            size_t size = list.size();
            for (size_t i = 0; i < size; ++i)
               if (list[i]->stub == &stub)
                  return { branch, i };
            break;
         }
      } else {
         auto* branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(&stub);
         if (branch_stub) {
            auto* branch = _item_for_branch_stub(*branch_stub);
            if (branch) {
               auto&  list = branch->topics;
               size_t size = list.size();
               for (size_t i = 0; i < size; ++i) {
                  auto* item = list[i];
                  if (item->stub == &stub)
                     return { branch, i };
               }
            }
            return none_result;
         }
      }
      auto&  list = this->_data.branchless_topics;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto* item = list[i];
         if (item->stub == &stub)
            return { nullptr, i };
      }
      return none_result;
   }
   
   dovah::form_stub* QuestAllDialogueDatastore::get_quest() const {
      return this->_quest;
   }
   void QuestAllDialogueDatastore::set_quest(dovah::form_stub* stub) {
      if (stub == this->_quest)
         return;
      this->_clear(true);
      this->_quest = stub;
      if (!stub)
         return;

      dovah::form_stub_helpers::for_each_quest_dialogue_branch(stub, [this](dovah::form_stub* branch_stub) {
         this->_add_branch_to_datastore(*branch_stub);
      });
      dovah::form_stub_helpers::for_each_quest_topic(stub, [this](dovah::form_stub* topic_stub) {
         auto* bs = dovah::form_stub_helpers::get_dialogue_topic_branch(topic_stub);
         if (bs)
            return;
         this->_add_branchless_topic_to_datastore(*topic_stub);
      });
      emit this->on_filled();
   }

   const std::vector<const QuestAllDialogueDatastore::Branch*>& QuestAllDialogueDatastore::all_branches() const {
      return reinterpret_cast<const std::vector<const Branch*>&>(this->_data.branches);
   }
   const std::vector<const QuestAllDialogueDatastore::Topic*>& QuestAllDialogueDatastore::all_branchless_topics() const {
      return reinterpret_cast<const std::vector<const Topic*>&>(this->_data.branchless_topics);
   }

   const QuestAllDialogueDatastore::Branch* QuestAllDialogueDatastore::item_for_branch_stub(const dovah::form_stub& stub) const {
      return _item_for_branch_stub(stub);
   }
   const QuestAllDialogueDatastore::Topic* QuestAllDialogueDatastore::item_for_topic_stub(const dovah::form_stub& stub) const {
      return _item_for_topic_stub(stub);
   }

   #pragma region Helper functions for form structure
      void QuestAllDialogueDatastore::reorder_info(const Info& info, int by) {
         if (!by)
            return;

         auto* topic = info.parent;
         assert(topic != nullptr);

         auto&  list = topic->infos;
         size_t size = list.size();
         bool   done = false;
         for (size_t i = 0; i < size; ++i) {
            if (list[i] == &info) {
               done = true;
               cobb::vectors::move_item_within<true>(list, i, by);
               break;
            }
         }
         if (!done)
            return;
         _sync_info_order_to_form(*topic);
         emit this->on_info_reordered(info);
      }
      void QuestAllDialogueDatastore::reorder_infos(const Topic& topic, size_t start, size_t count, int by) {
         if (!by || !count)
            return;

         auto&  list = topic.infos;
         size_t size = list.size();
         assert(start + count < size && "The requested range extends past the end of the list.");
         if (by < 0) {
            if (start < -by) {
               by = -(int)start;
               if (by == 0)
                  return;
            }
         } else {
            size_t last = start + count - 1;
            if (last + by >= size) {
               size_t first = size - count;
               assert(first >= start); // Should be impossible unless the `start + count < size` precondition also failed.
               if (first == start)
                  return;
               by = first - start;
            }
         }
         cobb::vectors::move_range(const_cast<Topic&>(topic).infos, start, count, start + by);
         _sync_info_order_to_form(topic);
         for (size_t i = 0; i < count; ++i) {
            auto* info = topic.infos[start + by + i];
            emit this->on_info_reordered(*info);
         }
      }
   #pragma endregion

   void QuestAllDialogueDatastore::_sync_info_order_to_form(const Topic& topic) {
      auto&  src_list = topic.infos;
      size_t src_size = src_list.size();

      auto* addenda = topic.stub->addenda;
      if (!addenda) {
         assert(src_size == 0 && "If QuestAllDialogueDatastore has INFOs in a DIAL, but the DIAL has no addenda, then something failed to properly maintain the DIAL's ordered-child list.");
      }
      
      auto&  dst_list = addenda->ordered_children;
      size_t dst_size = src_list.size();
      assert(dst_size >= src_size);

      std::decay_t<decltype(dst_list)> replacement;
      replacement.resize(dst_size);
      for (size_t i = 0; i < src_size; ++i) {
         replacement[i] = src_list[i]->stub;
      }
      {
         size_t dst_i = src_size;
         for (size_t i = 0; i < dst_size; ++i) {
            auto* stub = dst_list[i];

            bool found = false;
            for (size_t j = 0; j < src_size; ++j) {
               if (replacement[j] == stub) {
                  found = true;
                  break;
               }
            }
            if (found)
               continue;

            replacement[dst_i++] = stub;
         }
         assert(dst_i == dst_size);
      }
      std::swap(addenda->ordered_children, replacement);
   }

   QuestAllDialogueDatastore::Topic* QuestAllDialogueDatastore::_make_datastore_item_for_topic(dovah::form_stub& stub) {
      auto* topic = new Topic;
      topic->stub = &stub;
      topic->recache_from_stub();
      
      auto _handle_info = [this, topic](dovah::form_stub* child) {
         if (child->form_type != dovah::form_type::topic_info)
            return;
         this->_add_info_to_datastore(*topic, *child);
      };

      if (const auto* addenda = stub.addenda) {
         for (auto* child : addenda->ordered_children) {
            _handle_info(child);
         }
      } else {
         dovah::form_stub_helpers::for_each_child_form(&stub, _handle_info);
      }
      return topic;
   }

   void QuestAllDialogueDatastore::_add_branch_to_datastore(dovah::form_stub& branch_stub) {
      dovah::form_stub* starting_topic = nullptr;

      auto* branch = this->_data.branches.emplace_back() = new Branch;
      branch->stub = &branch_stub;
      {
         auto loaded = branch->stub->load().ptr_cast<dovah::loaded_forms::DialogueBranch>();
         branch->recache_from_stub();
         if (loaded)
            starting_topic = loaded->starting_topic.get_form_stub();
      }

      dovah::form_stub_helpers::for_each_dialogue_branch_topic(&branch_stub, [this, branch, starting_topic](dovah::form_stub* topic_stub) {
         this->_add_topic_to_datastore(*branch, *topic_stub);

         auto* topic = branch->topics.back();
         if (topic->stub == starting_topic) {
            branch->starting_topic = topic;
         }
      });
      if (!branch->starting_topic && !branch->topics.empty()) {
         branch->starting_topic = branch->topics[0];
      }
   }
   void QuestAllDialogueDatastore::_add_branchless_topic_to_datastore(dovah::form_stub& stub) {
      auto*& ptr = this->_data.branchless_topics.emplace_back();
      ptr = this->_make_datastore_item_for_topic(stub);
   }
   void QuestAllDialogueDatastore::_add_topic_to_datastore(Branch& branch, dovah::form_stub& topic_stub) {
      auto*& ptr = branch.topics.emplace_back();
      ptr = this->_make_datastore_item_for_topic(topic_stub);
      ptr->parent = &branch;
   }
   void QuestAllDialogueDatastore::_add_info_to_datastore(Topic& topic, dovah::form_stub& stub) {
      auto* info = topic.infos.emplace_back() = new Info;
      info->stub   = &stub;
      info->parent = &topic;
      info->recache_from_stub(*this->_quest);
   }

   void QuestAllDialogueDatastore::_remove_branch_from_datastore(size_t i) {
      auto& list = this->_data.branches;
      assert(i <= list.size());
      auto* item = list[i];
      list.erase(list.begin() + i);
      try {
         emit on_branch_removed(*item);
         delete item;
      } catch (...) {
         delete item;
         throw;
      }
   }
   void QuestAllDialogueDatastore::_remove_branchless_topic_from_datastore(size_t i) {
      auto& list = this->_data.branchless_topics;
      assert(i <= list.size());
      auto* item = list[i];
      list.erase(list.begin() + i);
      try {
         emit on_topic_removed(*item);
         delete item;
      } catch (...) {
         delete item;
         throw;
      }
   }
   void QuestAllDialogueDatastore::_remove_topic_from_datastore(Branch& branch, size_t i) {
      auto& list = branch.topics;
      assert(i <= list.size());
      auto* item = list[i];
      list.erase(list.begin() + i);
      try {
         assert(item->parent == &branch);
         emit on_topic_removed(*item);
         delete item;
      } catch (...) {
         delete item;
         throw;
      }
   }
   void QuestAllDialogueDatastore::_remove_info_from_datastore(Topic& topic, size_t i) {
      auto& list = topic.infos;
      assert(i <= list.size());
      auto* item = list[i];
      list.erase(list.begin() + i);
      try {
         assert(item->parent == &topic);
         emit on_info_removed(*item);
         delete item;
      } catch (...) {
         delete item;
         throw;
      }
   }

   #pragma region Handlers
      void QuestAllDialogueDatastore::_handle_branch_edited(dovah::form_stub& stub) {
         auto&  list = this->_data.branches;
         size_t size = list.size();

         auto* quest_stub = dovah::form_stub_helpers::get_dialogue_branch_quest(&stub);
         if (quest_stub != this->_quest) {
            //
            // Handle the case of one of our branches being moved to another quest.
            //
            for (size_t i = 0; i < size; ++i) {
               if (list[i]->stub == &stub) {
                  this->_remove_branch_from_datastore(i);
                  break;
               }
            }
            return;
         }
         //
         // One of our branches was modified.
         //
         for (size_t i = 0; i < size; ++i) {
            auto* item = list[i];
            if (item->stub != &stub)
               continue;
            item->recache_from_stub();
            emit this->on_branch_edited(*item);
            return;
         }
         //
         // A branch was moved into our quest.
         //
         this->_add_branch_to_datastore(stub);
         auto* item = this->_data.branches.back();
         assert(item && item->stub == &stub);
         emit this->on_branch_added(*item);
      }
      void QuestAllDialogueDatastore::_handle_topic_edited(dovah::form_stub& stub) {
         if (!this->_quest || dovah::form_stub_helpers::get_dialogue_topic_quest(&stub) != this->_quest)
            return;

         Branch* branch_prior = nullptr;
         Branch* branch_after = nullptr;
         size_t  index_prior  = -1;
         Topic*  topic_item   = nullptr;
         {  //
            // Find where in the datastore the topic is or should be, and check to 
            // see if it was moved across branches.
            //
            auto* branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(&stub);
            for (auto* branch : this->_data.branches) {
               if (branch->stub == branch_stub)
                  branch_after = branch;

               auto&  list = branch->topics;
               size_t size = list.size();
               for (size_t i = 0; i < size; ++i) {
                  if (list[i]->stub == &stub) {
                     topic_item   = list[i];
                     branch_prior = branch;
                     index_prior  = i;
                     break;
                  }
               }
               if (branch_prior && branch_after)
                  break;
            }
            if (!branch_after) {
               //
               // It may be a branchless topic. Check.
               //
               for (auto* topic : this->_data.branchless_topics) {
                  if (topic->stub == &stub) {
                     topic_item = topic;
                     break;
                  }
               }
            }
         }
         if (branch_prior != branch_after) {
            //
            // Topic moved across branches.
            //
            if (branch_prior) {
               if (!branch_after) {
                  //
                  // Topic moved to a different quest?!
                  //
                  this->_remove_topic_from_datastore(*branch_prior, index_prior);
                  return;
               }
               //
               // Remove from branch_prior:
               //
               {
                  auto& list = branch_prior->topics;
                  list.erase(list.begin() + index_prior);
                  emit this->on_topic_removed(*topic_item);
               }
               //
               // Add to branch_after:
               //
               {
                  topic_item->parent = branch_after;
                  topic_item->recache_from_stub();
                  branch_after->topics.push_back(topic_item);
                  emit this->on_topic_added(*topic_item);
               }
            } else if (branch_after) {
               //
               // Topic moved in from a different quest?!
               //
               this->_add_topic_to_datastore(*branch_after, stub);
               topic_item = branch_after->topics.back();
               assert(topic_item && topic_item->stub == &stub);
               emit this->on_topic_added(*topic_item);
            }
            return;
         }
         if (!branch_after && !topic_item) {
            //
            // Branchless topic moved in from a different quest?!
            //
            this->_add_branchless_topic_to_datastore(stub);
            topic_item = this->_data.branchless_topics.back();
            assert(topic_item && topic_item->stub == &stub);
            emit this->on_topic_added(*topic_item);
            return;
         }
         //
         // One of our topics was modified.
         //
         assert(topic_item != nullptr);
         topic_item->recache_from_stub();
         emit this->on_topic_edited(*topic_item);
      }
      void QuestAllDialogueDatastore::_handle_info_edited(dovah::form_stub& stub) {
         Topic* topic_prior = nullptr;
         Topic* topic_after = nullptr;
         size_t index_prior = -1;
         Info*  info_item   = nullptr;
         {
            auto* topic_stub = stub.get_parent_form();
            if (topic_stub->form_type != dovah::form_type::topic)
               topic_stub = nullptr;

            auto _handle_topic = [&topic_prior, &topic_after, &index_prior, &info_item, topic_stub, &stub](Topic* topic) {
               if (topic->stub == topic_stub) {
                  topic_after = topic;
               }
               auto idx = topic->index_of_info(stub);
               if (idx != (size_t)-1) {
                  topic_prior = topic;
                  index_prior = idx;
                  info_item   = topic->infos[idx];
               }
            };
            auto _done = [&topic_prior, &topic_after]() {
               return (topic_prior && topic_after);
            };

            for (auto* topic : this->_data.branchless_topics) {
               _handle_topic(topic);
               if (_done())
                  break;
            }
            if (!topic_prior || !topic_after) {
               for (auto* branch : this->_data.branches) {
                  for (auto* topic : branch->topics) {
                     _handle_topic(topic);
                     if (_done())
                        break;
                  }
                  if (_done())
                     break;
               }
            }
         }
         if (topic_prior != topic_after) {
            //
            // Info moved across topics.
            //
            if (!info_item) {
               //
               // Info moved to a different quest?!
               //
               if (!topic_prior)
                  return;
               this->_remove_info_from_datastore(*topic_prior, index_prior);
               return;
            }
            //
            // Remove from topic_prior:
            //
            {
               auto& list = topic_prior->infos;
               list.erase(list.begin() + index_prior);
               emit this->on_info_removed(*info_item);
            }
            //
            // Add to topic_after:
            //
            {
               info_item->parent = topic_after;
               info_item->recache_from_stub(*this->_quest);
               topic_after->infos.push_back(info_item);
               emit this->on_info_added(*info_item);
            }
            return;
         }
         //
         // One of our infos was modified.
         //
         assert(info_item != nullptr);
         info_item->recache_from_stub(*this->_quest);
         emit this->on_info_edited(*info_item);
      }

      void QuestAllDialogueDatastore::_handle_branch_created(dovah::form_stub& stub) {
         if (this->_quest != dovah::form_stub_helpers::get_dialogue_branch_quest(&stub))
            return;

         this->_add_branch_to_datastore(stub);
         auto* item = this->_data.branches.back();
         assert(item && item->stub == &stub);
         emit this->on_branch_added(*item);
      }
      void QuestAllDialogueDatastore::_handle_topic_created(dovah::form_stub& stub) {
         if (this->_quest != dovah::form_stub_helpers::get_dialogue_topic_quest(&stub))
            return;

         Branch* parent  = nullptr;
         Topic*  created = nullptr;
         {
            auto* branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(&stub);
            if (branch_stub)
               parent = this->_item_for_branch_stub(*branch_stub);
         }
         if (parent) {
            this->_add_topic_to_datastore(*parent, stub);
            created = parent->topics.back();
         } else {
            this->_add_branchless_topic_to_datastore(stub);
            created = this->_data.branchless_topics.back();
         }
         assert(created && created->stub == &stub);
         assert(created->parent == parent);
         emit this->on_topic_added(*created);
      }
      void QuestAllDialogueDatastore::_handle_info_created(dovah::form_stub& stub) {
         auto* topic_stub = stub.get_parent_form();
         if (!topic_stub || topic_stub->form_type != dovah::form_type::topic)
            return;
         if (this->_quest != dovah::form_stub_helpers::get_dialogue_topic_quest(topic_stub))
            return;

         Topic* parent = _item_for_topic_stub(*topic_stub);
         if (!parent)
            return;

         this->_add_info_to_datastore(*parent, stub);
         Info* created = parent->infos.back();
         assert(created && created->stub == &stub);
         assert(created->parent == parent);
         emit this->on_info_added(*created);
      }

      void QuestAllDialogueDatastore::_handle_branch_removed(dovah::form_stub& stub) {
         auto&  list = this->_data.branches;
         size_t size = list.size();
         for (size_t i = 0; i < size; ++i) {
            if (list[i]->stub == &stub) {
               this->_remove_branch_from_datastore(i);
               break;
            }
         }
      }
      void QuestAllDialogueDatastore::_handle_topic_removed(dovah::form_stub& stub) {
         auto   pair   = _locate_topic(stub);
         auto*  branch = pair.first;
         size_t i      = pair.second;
         if (branch == nullptr) {
            if (i == (size_t)-1)
               return;
            this->_remove_branchless_topic_from_datastore(i);
            return;
         }
         assert(i != (size_t)-1);
         this->_remove_topic_from_datastore(*branch, i);
      }
      void QuestAllDialogueDatastore::_handle_info_deleted(dovah::form_stub& stub, bool just_being_flagged) {
         auto* topic_stub = stub.get_parent_form();
         if (!topic_stub)
            return;
         if (this->_quest != dovah::form_stub_helpers::get_dialogue_topic_quest(topic_stub))
            return;
         auto* topic = this->_item_for_topic_stub(*topic_stub);
         if (!topic)
            return;
         
         auto&  list = topic->infos;
         size_t size = list.size();
         for (size_t i = 0; i < size; ++i) {
            auto* item = list[i];
            if (item->stub != &stub)
               continue;
            if (just_being_flagged) {
               item->deleted = true;
               emit this->on_info_edited(*item);
               return;
            } else {
               this->_remove_info_from_datastore(*topic, i);
               return;
            }
         }
      }

      void QuestAllDialogueDatastore::_handle_form_deleted(dovah::form_stub& stub, bool just_being_flagged) {
         if (!this->_quest)
            return;
         switch (stub.form_type) {
            case dovah::form_type::quest:
               if (this->_quest == &stub) {
                  this->_quest = nullptr;
                  this->_clear(false);
               }
               return;
            case dovah::form_type::dialogue_branch:
               this->_handle_branch_removed(stub);
               return;
            case dovah::form_type::topic:
               this->_handle_topic_removed(stub);
               return;
            case dovah::form_type::topic_info:
               this->_handle_info_deleted(stub, just_being_flagged);
               return;
         }
      }
   #pragma endregion
#pragma endregion