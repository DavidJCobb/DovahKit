#include "./TopicInfoWalkAwayTopicFormFilter.h"
#include "dovah/form_stub.h"

/*virtual*/ bool TopicInfoWalkAwayTopicFormFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   if (stub.form_type != dovah::form_type::topic)
      return false;
   if (!this->_model)
      return false;
   if (!this->_model->containsTopic(&stub))
      return false;
   return true;
}

void TopicInfoWalkAwayTopicFormFilter::setSourceModel(TopicInfoLinkedTopicsModel* model) {
   if (model == this->_model)
      return;
   if (auto* prior = this->_model.data()) {
      QObject::disconnect(prior, nullptr, this, nullptr);
   }
   this->_model = model;
   if (model) {
      QObject::connect(model, &QAbstractItemModel::modelReset,   this, &TopicInfoWalkAwayTopicFormFilter::_refilter_all_forms);
      QObject::connect(model, &QAbstractItemModel::rowsInserted, this, &TopicInfoWalkAwayTopicFormFilter::_refilter_all_forms);
      QObject::connect(model, &QAbstractItemModel::rowsRemoved,  this, &TopicInfoWalkAwayTopicFormFilter::_refilter_all_forms);
   }
   this->_refilter_all_forms();
}