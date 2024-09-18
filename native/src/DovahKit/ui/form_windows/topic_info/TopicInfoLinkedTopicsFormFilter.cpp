#include "./TopicInfoLinkedTopicsFormFilter.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_quest.h"
#include "dovah/form_stub.h"

/*virtual*/ bool TopicInfoLinkedTopicsFormFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   if (stub.form_type != dovah::form_type::topic)
      return false;
   if (!this->_model || !this->_quest)
      return false;
   if (dovah::form_stub_helpers::get_dialogue_topic_quest(&stub) != this->_quest)
      return false;
   if (this->_model->containsTopic(&stub))
      return false;
   return true;
}

void TopicInfoLinkedTopicsFormFilter::setModel(TopicInfoLinkedTopicsModel* model) {
   if (model == this->_model)
      return;
   if (auto* prior = this->_model.data()) {
      QObject::disconnect(prior, nullptr, this, nullptr);
   }
   this->_model = model;
   if (model) {
      QObject::connect(model, &QAbstractItemModel::modelReset,   this, &TopicInfoLinkedTopicsFormFilter::_refilter_all_forms);
      QObject::connect(model, &QAbstractItemModel::rowsInserted, this, &TopicInfoLinkedTopicsFormFilter::_refilter_all_forms);
      QObject::connect(model, &QAbstractItemModel::rowsRemoved,  this, &TopicInfoLinkedTopicsFormFilter::_refilter_all_forms);
   }
   this->_refilter_all_forms();
}
void TopicInfoLinkedTopicsFormFilter::setOwningQuest(const dovah::form_stub* stub) {
   if (stub == this->_quest)
      return;
   this->_quest = stub;
   this->_refilter_all_forms();
}