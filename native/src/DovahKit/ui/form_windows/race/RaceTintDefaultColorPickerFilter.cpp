#include "./RaceTintDefaultColorPickerFilter.h"
#include "dovah/form_stub.h"

/*virtual*/ bool RaceTintDefaultColorPickerFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   if (!this->_model || !this->_layer.isValid() || this->_layer.model() != this->_model)
      return true;
   return this->_model->layer_has_color(this->_layer, stub);
}

void RaceTintDefaultColorPickerFilter::setModel(RaceTintLayerModel* model) {
   if (this->_model == model)
      return;

   if (RaceTintLayerModel* prior = this->_model) {
      QObject::disconnect(prior, nullptr, this, nullptr);
   }
   this->_model = model;
   if (model) {
      QObject::connect(model, &QAbstractItemModel::rowsInserted, this, &RaceTintDefaultColorPickerFilter::_on_model_structure_change);
      QObject::connect(model, &QAbstractItemModel::rowsRemoved,  this, &RaceTintDefaultColorPickerFilter::_on_model_structure_change);
      QObject::connect(model, &QAbstractItemModel::dataChanged,  this, &RaceTintDefaultColorPickerFilter::_on_model_data_change);
   }
   this->_refilter_all_forms();
}
void RaceTintDefaultColorPickerFilter::setLayer(const QModelIndex& qmi) {
   if (this->_layer == qmi)
      return;
   this->_layer = qmi;
   this->_refilter_all_forms();
}

void RaceTintDefaultColorPickerFilter::_on_model_structure_change(const QModelIndex& parent) {
   if (!this->_layer.isValid())
      return;

   if (parent.isValid())
      //
      // Top-level insertion/deletion, i.e. of a layer, not a preset.
      //
      return;
   if (parent != this->_layer)
      //
      // Insertion/deletion in a different layer than the one we're interested in.
      //
      return;

   this->_refilter_all_forms();
}
void RaceTintDefaultColorPickerFilter::_on_model_data_change(const QModelIndex& subject) {
   if (!this->_layer.isValid())
      return;

   auto parent = this->_model->parent(subject);
   if (!parent.isValid() || parent != this->_layer)
      return;

   this->_refilter_all_forms();
}