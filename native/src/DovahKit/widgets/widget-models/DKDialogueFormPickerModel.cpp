#include "./DKDialogueFormPickerModel.h"
#include "dovah/data/dialogue/topic_subtype.h"
#include "dovah/form_stubs/helpers/for_each_quest_dialogue_branch.h"
#include "dovah/form_stubs/helpers/for_each_quest_topic.h"
#include "dovah/form_stubs/helpers/get_dialogue_branch_quest.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_branch.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_quest.h"
#include "dovah/forms/Topic.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/core.h"

namespace {
   constexpr const size_t no_subtype = (size_t)-1;

   size_t _subtype_index_of(dovah::form_stub* topic) {
      auto loaded = topic->load().ptr_cast<dovah::loaded_forms::Topic>();
      if (!loaded)
         return no_subtype;

      const auto* info = dovah::dialogue::topic_subtype_by_signature(loaded->subtype);
      if (info)
         return dovah::dialogue::topic_subtype_index(*info);
      return no_subtype;
   }

   bool _quest_has_topic_with_subtype(const dovah::form_stub* quest, size_t index) {
      bool found = false;
      dovah::form_stub_helpers::for_each_quest_topic(quest, [index, &found](dovah::form_stub* topic) -> bool {
         auto i = _subtype_index_of(topic);
         if (i == index)
            return (found = true);
         return false;
      });
      return found;
   }
}

void DKDialogueFormPickerModelNode::recache() {
   if (auto* stub = this->branch)
      this->cached.branch_editor_id = QString::fromStdString(stub->editorID);
   if (auto* stub = this->scene)
      this->cached.scene_editor_id = QString::fromStdString(stub->editorID);
   this->recache_topic();
}
void DKDialogueFormPickerModelNode::recache_topic() {
   if (auto* stub = this->topic) {
      this->cached.topic_editor_id = QString::fromStdString(stub->editorID);
      this->subtype = _subtype_index_of(stub);
   } else {
      this->subtype = (size_t)-1;
   }
}
void DKDialogueFormPickerModelNode::update_quest_if_transplanted() {
   dovah::form_stub* quest = nullptr;
   if (this->topic)
      quest = dovah::form_stub_helpers::get_dialogue_topic_quest(this->topic);
   else if (this->branch)
      quest = dovah::form_stub_helpers::get_dialogue_branch_quest(this->topic);
   else
      return;

   if (quest == this->quest)
      return;
   this->quest = quest;

   auto& dst = this->cached.quest_editor_id;
   if (branch)
      dst = QString::fromStdString(quest->editorID);
   else
      dst.clear();
}
void DKDialogueFormPickerModelNode::update_branch_if_transplanted() {
   if (!this->topic)
      return;

   auto* branch = dovah::form_stub_helpers::get_dialogue_topic_branch(this->topic);
   if (branch == this->branch)
      return;
   this->branch = branch;

   auto& dst = this->cached.branch_editor_id;
   if (branch)
      dst = QString::fromStdString(branch->editorID);
   else
      dst.clear();
}

DKDialogueFormPickerModel::DKDialogueFormPickerModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &DKDialogueFormPickerModel::clear);
   QObject::connect(&editor, &DovahKitCore::formModified, this, &DKDialogueFormPickerModel::_on_form_modified);
   QObject::connect(&editor, &DovahKitCore::formRenumbered, this, &DKDialogueFormPickerModel::_on_form_renumbered);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &DKDialogueFormPickerModel::_on_form_deleted);
}

#pragma region Overrides
   QVariant DKDialogueFormPickerModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      dovah::form_stub* form = node.innermost_form();
      switch (column) {
         case Column::Quest:
            if (node.quest == this->quest) {
               return this->cached.quest_editor_id;
            }
            return node.cached.quest_editor_id;
         case Column::BranchOrScene:
            if (node.branch)
               return node.cached.branch_editor_id;
            return node.cached.scene_editor_id;
         case Column::Topic:
            return node.cached.topic_editor_id;
         case Column::FormID:
            return editor_helpers::form_id_to_string(form ? form->formID : 0);
         case Column::Subtype:
            if (node.subtype != no_subtype) {
               auto& name = dovah::dialogue::all_topic_subtypes[node.subtype].internal_name;
               return QString::fromLatin1(name.data(), name.size());
            }
            break;
      }
      return {};
   }
   Qt::ItemFlags DKDialogueFormPickerModel::flags_of(const node_type&, size_t column) const {
      auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
      return flags;
   }

   /*virtual*/ QVariant DKDialogueFormPickerModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (role != Qt::DisplayRole)
         return {};
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      switch (section) {
         using enum Column::enumeration;
         case Quest:
            return tr("Quest");
         case BranchOrScene:
            return tr("Branch/Scene");
         case Topic:
            return tr("Topic");
         case FormID:
            return tr("Form ID");
         case Subtype:
            return tr("Subtype");
      }
      return {};
   }
#pragma endregion
   
void DKDialogueFormPickerModel::clear() {
   this->desired_type = dovah::form_type::none;
   this->quest        = nullptr;
   this->form_to_link = nullptr;
   this->form_to_move = nullptr;
   this->cached       = {};

   for (auto* node : this->_nodes) {
      delete node;
   }
   this->_nodes.clear();
}

void DKDialogueFormPickerModel::setDesiredFormType(dovah::form_type d) {
   this->desired_type = d;
}
void DKDialogueFormPickerModel::setQuest(dovah::form_stub* stub) {
   this->quest = stub;
}
void DKDialogueFormPickerModel::setFormToLink(dovah::form_stub* stub) {
   this->form_to_link = stub;
}
void DKDialogueFormPickerModel::setFormToMove(dovah::form_stub* stub) {
   this->form_to_move = stub;
}

void DKDialogueFormPickerModel::refill() {
   this->performReset([this]() {

      dovah::form_stub* exclude = nullptr;
      size_t exclude_subtype = no_subtype;
      
      switch (this->desired_type) {
         case dovah::form_type::quest:
            if (auto* stub = this->form_to_move) {
               switch (stub->form_type) {
                  case dovah::form_type::dialogue_branch:
                     exclude = dovah::form_stub_helpers::get_dialogue_branch_quest(stub);
                     break;
                  case dovah::form_type::topic:
                     exclude         = dovah::form_stub_helpers::get_dialogue_topic_quest(stub);
                     exclude_subtype = _subtype_index_of(stub);
                     if (exclude_subtype != no_subtype) {
                        const auto& info = dovah::dialogue::all_topic_subtypes[exclude_subtype];
                        if (info.is_reusable)
                           exclude_subtype = no_subtype;
                     }
                     break;
               }
            }
            DovahKitCore::get().for_each_form_of_type(dovah::form_type::quest, [this, exclude, exclude_subtype](dovah::form_stub* stub) {
               if (stub == exclude)
                  return false;
               if (exclude_subtype != no_subtype) {
                  if (_quest_has_topic_with_subtype(stub, exclude_subtype))
                     return false;
               }
               auto* node = new node_type;
               this->_nodes.push_back(node);
               node->quest = stub;
               node->cached.quest_editor_id = QString::fromStdString(stub->editorID);
               return false;
            });
            break;
         case dovah::form_type::dialogue_branch:
            if (!this->quest)
               return;
            if (auto* stub = this->form_to_move) {
               if (stub->form_type == dovah::form_type::topic)
                  exclude = dovah::form_stub_helpers::get_dialogue_topic_quest(stub);
            }
            dovah::form_stub_helpers::for_each_quest_dialogue_branch(this->quest, [this, exclude](dovah::form_stub* stub) {
               if (stub == exclude)
                  return;
               auto* node = new node_type;
               this->_nodes.push_back(node);
               node->quest  = this->quest;
               node->branch = stub;
               node->recache();
            });
            break;
         case dovah::form_type::topic:
            if (!this->quest)
               return;
            if (auto* stub = this->form_to_move) {
               if (stub->form_type == dovah::form_type::topic_info)
                  exclude = stub->get_parent_form();
            }
            dovah::form_stub_helpers::for_each_quest_topic(this->quest, [this, exclude](dovah::form_stub* stub) {
               if (stub == exclude)
                  return;
               auto* node = new node_type;
               node->quest  = this->quest;
               node->branch = dovah::form_stub_helpers::get_dialogue_topic_branch(stub);
               node->topic  = stub;
               node->recache();
               this->_nodes.push_back(node);
            });
            break;
      }
   });
}

void DKDialogueFormPickerModel::_on_form_modified(dovah::form_stub* stub) {
   if (stub == this->quest) {
      this->cached.quest_editor_id = QString::fromStdString(stub->editorID);

      auto tl = this->index(0, Column::Quest, {});
      auto br = this->index(this->_nodes.size() - 1, Column::Quest, {});
      emit dataChanged(tl, br);
      return;
   }

   size_t column = -1;
   switch (stub->form_type) {
      case dovah::form_type::quest:
         column = Column::Quest;
         break;
      case dovah::form_type::scene:
         [[fallthrough]];
      case dovah::form_type::dialogue_branch:
         column = Column::BranchOrScene;
         break;
      case dovah::form_type::topic:
         column = Column::Topic;
         break;
      default:
         return;
   }

   QString editor_id;
   auto _get_editor_id = [&editor_id, stub]() -> const QString& {
      if (editor_id.isEmpty())
         editor_id = QString::fromStdString(stub->editorID);
      return editor_id;
   };

   for (size_t i = 0; i < this->_nodes.size(); ++i) {
      auto& node    = *this->_nodes[i];
      bool  changed = false;
      if (node.quest == stub) {
         node.cached.quest_editor_id = _get_editor_id();
         changed = true;
      } else if (node.branch == stub) {
         node.cached.branch_editor_id = _get_editor_id();
         node.update_quest_if_transplanted();
         changed = true;
      } else if (node.scene == stub) {
         node.cached.scene_editor_id = _get_editor_id();
         node.update_quest_if_transplanted(); // TODO: 9/18/2024: not implemented for scenes yet
         changed = true;
      } else if (node.topic == stub) {
         node.recache_topic();
         node.update_branch_if_transplanted();
         node.update_quest_if_transplanted();
         changed = true;
      }
      if (changed) {
         auto qmi = this->index(i, column, {});
         emit dataChanged(qmi, qmi);
      }
   }
}
void DKDialogueFormPickerModel::_on_form_renumbered(dovah::form_stub* stub) {
   if (stub == this->quest)
      return;
   if (stub->form_type != this->desired_type)
      return;
   for (size_t i = 0; i < this->_nodes.size(); ++i) {
      if (this->_nodes[i]->innermost_form() == stub) {
         auto qmi = this->index(i, Column::FormID, {});
         emit dataChanged(qmi, qmi);
      }
   }
}
void DKDialogueFormPickerModel::_on_form_deleted(dovah::form_stub* stub) {
   if (stub == this->quest) {
      this->clear();
      return;
   }
   if (stub->form_type != this->desired_type)
      return;

   size_t size = this->_nodes.size();
   for (size_t i = 0; i < size; ++i) {
      auto* node = this->_nodes[i];
      if (node->innermost_form() == stub) {
         this->beginRemoveRows({}, i, i);
         this->_nodes.erase(this->_nodes.begin() + i);
         delete node;
         --i;
         --size;
         this->endRemoveRows();
      }
   }
}