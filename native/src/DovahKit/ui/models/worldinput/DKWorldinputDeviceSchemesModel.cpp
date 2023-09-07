#include "./DKWorldinputDeviceSchemesModel.h"
#include <cassert>
#include <QDirIterator>
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"
#include "editor/subsystems/options/core.h"

namespace {
   using control_scheme_manager = dovahkit::subsystems::worldinput2::control_scheme_manager;
}

DKWorldinputDeviceSchemesModel::DKWorldinputDeviceSchemesModel(QObject* parent) : DKGenericListModel(parent) {
   auto& mgr = control_scheme_manager::get_or_create();
   
   QObject::connect(&mgr, &control_scheme_manager::beforeReset, this, [this]() {
      this->clear();
   });
   QObject::connect(&mgr, &control_scheme_manager::reloadedAll, this, [this]() {
      this->reload(this->_last_used_device_type);
   });
}

QVariant DKWorldinputDeviceSchemesModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   if (!node.src)
      return {};
   //
   switch (role) {
      case IsHardcodedRole:
         return node.src->is_hardcoded();
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         return node.src->name;
   }
   return {};
}
Qt::ItemFlags DKWorldinputDeviceSchemesModel::flags_of(const node_type& node, size_t column) const {
   if (!node.src)
      return {};
   Qt::ItemFlags flags = {};
   flags |= Qt::ItemFlag::ItemIsEnabled;
   flags |= Qt::ItemFlag::ItemIsSelectable;
   return flags;
}

void DKWorldinputDeviceSchemesModel::reload(input_device_type dt) {
   this->_last_used_device_type = dt;
   this->performReset([this, dt]() {
      auto& mgr = dovahkit::subsystems::worldinput2::control_scheme_manager::get_or_create();

      const auto& list = mgr.schemes_by_device(dt);
      this->_nodes.reserve(list.size());
      for (auto* item : list) {
         if (item->device_type != dt)
            continue;
         auto* node = new node_type;
         this->_nodes.push_back(node);
         node->src  = item;
      }
      this->_nodes.shrink_to_fit();
   });
}

int DKWorldinputDeviceSchemesModel::rowFor(const data_type& d) {
   static_assert(std::is_base_of_v<data_type, node_type::saved_data_type>);
   for (size_t i = 0; i < this->_nodes.size(); ++i) {
      auto* item = this->_nodes[i];
      if (item->src == &d)
         return i;
   }
   return -1;
}

std::optional<DKWorldinputDeviceSchemesModel::data_type> DKWorldinputDeviceSchemesModel::dataFor(const QModelIndex& qmi) const {
   if (auto* node = this->node(qmi))
      return *(node->src);
   return {};
}
void DKWorldinputDeviceSchemesModel::replaceDataFor(const QModelIndex& qmi, const data_type& data) {
   if (auto* node = this->node(qmi)) {
      assert(node->src);

      auto& mgr = dovahkit::subsystems::worldinput2::control_scheme_manager::get();
      mgr.overwrite_scheme(node->src, data);

      this->emitNodeChanged(qmi);
   }
}

const DKWorldinputDeviceSchemesModel::saved_data_type* DKWorldinputDeviceSchemesModel::savedSchemeAtRow(int r) const {
   if (r < 0 || r >= this->_nodes.size())
      return nullptr;
   return this->_nodes[r]->src;
}

QModelIndex DKWorldinputDeviceSchemesModel::insert(data_type data) {
   auto& mgr = dovahkit::subsystems::worldinput2::control_scheme_manager::get();
   mgr.adjust_scheme_name_by_availability(data.device_type, data.name);
   const auto* src = mgr.add_scheme(data);
   if (!src)
      return {};

   this->beginInsertRows({}, this->_nodes.size(), this->_nodes.size());
   //
   auto* node = new node_type;
   this->_nodes.push_back(node);
   node->src = src;
   //
   this->endInsertRows();
   
   return index(node);
}

bool DKWorldinputDeviceSchemesModel::removeRows(int row, int count, const QModelIndex& parent) {
   if (parent.isValid())
      return false;

   if (row < 0)
      return false;
   if (count < 1 || row + count >= this->_nodes.size())
      return false;
   auto* node = this->_nodes[row];
   if (!node)
      return false;

   assert(node->src);

   auto& mgr = dovahkit::subsystems::worldinput2::control_scheme_manager::get();
   mgr.delete_scheme(node->src);
   return this->deleteItems(row, count);
}