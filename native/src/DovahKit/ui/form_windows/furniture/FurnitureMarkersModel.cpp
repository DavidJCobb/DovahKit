#include "./FurnitureMarkersModel.h"
#include <memory>
#include "dovah/form_stub.h"
#include "dovah/forms/Furniture.h"
#include "editor/core.h"
#pragma region NIF-related includes
   #include "dovah/files/bsa/bsa_archived_file.h"
   #include "editor/subsystems/assets.h"
   #include "nif/blocks/BSFurnitureMarkerNode.h"
   #include "nif/blocks/NiObjectNET.h"
   #include "nif/file.h"
#pragma endregion

#pragma region FurnitureMarkersModel
   FurnitureMarkersModel::FurnitureMarkersModel(QObject* parent) : QAbstractItemModel(parent) {
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
         if (stub->form_type != dovah::form_type::keyword)
            return;
         QString editor_id;
         for (size_t i = 0; i < this->_nodes.size(); ++i) {
            auto& node = this->_nodes[i];
            if (node.keyword != stub)
               continue;
            if (editor_id.isEmpty())
               editor_id = QString::fromStdString(stub->editorID);
            node.cached.keyword_editor_id = editor_id;

            auto qmi = this->index(i, 0, {});
            emit dataChanged(qmi, qmi, { KeywordRole });
         }
      });
      QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
         if (stub->form_type != dovah::form_type::keyword)
            return;
         for (size_t i = 0; i < this->_nodes.size(); ++i) {
            auto& node = this->_nodes[i];
            if (node.keyword != stub)
               continue;
            node.keyword = nullptr;
            node.cached.keyword_editor_id.clear();

            auto qmi = this->index(i, 0, {});
            emit dataChanged(qmi, qmi, { KeywordRole });
         }
      });
   }
   
   #pragma region QAbstractItemModel /*override*/s
      #pragma region Hierarchy
         /*virtual*/ QModelIndex FurnitureMarkersModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
            if (parent.isValid())
               return {};
            if (column != 0 || row < 0 || row >= this->_nodes.size())
               return {};
            return this->createIndex(row, column, nullptr);
         }
         /*virtual*/ QModelIndex FurnitureMarkersModel::parent(const QModelIndex& index) const /*override*/ {
            return {};
         }
         /*virtual*/ int FurnitureMarkersModel::rowCount(const QModelIndex& parent) const /*override*/ {
            return this->_nodes.size();
         }
         /*virtual*/ int FurnitureMarkersModel::columnCount(const QModelIndex& parent) const /*override*/ {
            return 1;
         }
      #pragma endregion
      #pragma region Node data
         /*virtual*/ QVariant FurnitureMarkersModel::data(const QModelIndex& index, int role) const /*override*/ {
            if (!index.isValid() || index.model() != this)
               return {};
            int row = index.row();
            if (row >= this->_nodes.size())
               return {};
            const auto& node = this->_nodes[row];
            switch (role) {
               case EntryPointsSupportedRole:
                  return (uint32_t)node.entry_points.supported;
               case EntryPointsEnabledRole:
                  return (uint32_t)node.entry_points.enabled;
               case KeywordRole:
                  return QVariant::fromValue(node.keyword);
               case AnimationTypeRole:
                  return (int)node.animation_type;
               case Qt::DisplayRole:
               case Qt::ToolTipRole:
                  {
                     QString type;
                     switch (node.animation_type) {
                        case AnimationType::sit:
                           type = tr("Sit");
                           break;
                        case AnimationType::sleep:
                           type = tr("Sleep");
                           break;
                        case AnimationType::lean:
                           type = tr("Lean");
                           break;
                     }
                     return tr("%1 %2").arg(type).arg(row);
                  }
                  break;
               case Qt::CheckStateRole:
                  if (row > 23) // only 24 flags in the form for this
                     return {};
                  if (node.enabled)
                     return (int)Qt::CheckState::Checked;
                  return Qt::CheckState::Unchecked;
            }
            return {};
         }
         /*virtual*/ Qt::ItemFlags FurnitureMarkersModel::flags(const QModelIndex& index) const /*override*/ {
            auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren;
            if (index.isValid() && index.row() < 23) {
               flags |= Qt::ItemFlag::ItemIsUserCheckable;
            }
            return flags;
         }
         #pragma region Write-access
            /*virtual*/ bool FurnitureMarkersModel::setData(const QModelIndex& index, const QVariant& value, int role) /*override*/ {
               if (!index.isValid() || index.model() != this)
                  return {};
               int row = index.row();
               if (row >= this->_nodes.size())
                  return {};
               auto& node = this->_nodes[row];
               switch (role) {
                  case EntryPointsEnabledRole:
                     if (!value.canConvert<int>())
                        return false;
                     node.entry_points.enabled.overwrite_with_raw_integer((uint8_t)value.toInt());
                     break;
                  case KeywordRole:
                     if (!value.canConvert<dovah::form_stub*>())
                        return false;
                     {
                        auto* stub = value.value<dovah::form_stub*>();
                        if (stub && stub->form_type != dovah::form_type::keyword)
                           return false;
                        node.keyword = stub;
                     }
                     break;
                  case AnimationTypeRole:
                     if (!value.canConvert<int>())
                        return false;
                     {
                        auto i = value.toInt();
                        if (i < 0 || i > 2)
                           return false;
                        node.animation_type = (AnimationType)i;
                     }
                     break;
                  case Qt::CheckStateRole:
                     if (!value.canConvert<int>())
                        return false;
                     {
                        auto state   = (Qt::CheckState)value.toInt();
                        bool result  = (state == Qt::CheckState::Checked);
                        if (node.enabled == result)
                           return true;
                        node.enabled = result;
                     }
                     break;
               }
               emit dataChanged(index, index, { role });
               return true;
            }
         #pragma endregion
      #pragma endregion
      /*virtual*/ QVariant FurnitureMarkersModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (section != 0)
            return {};
         if (role != Qt::DisplayRole)
            return {};
         return tr("Marker");
      }
   #pragma endregion

   void FurnitureMarkersModel::importData(const dovah::loaded_forms::Furniture& src_form) {
      this->beginResetModel();
      this->_nodes.clear();
      {
         const auto& src_list = src_form.marker_nif_infos;
         for (size_t i = 0; i < src_form.marker_nif_infos.size(); ++i) {
            const auto& src_item = src_list[i];
            auto&       dst_item = this->_nodes.emplace_back();
            dst_item.entry_points.supported.overwrite_with_raw_integer(~src_item.supported_entry);
            for (size_t i = 0; i < 3; ++i) {
               if (src_item.supported_animations & (1 << i)) {
                  dst_item.animation_type = (AnimationType)i;
                  break;
               }
            }
         }
      }
      if (!src_form.model.model_path.empty()) {
         this->_pull_marker_info_from_nif(src_form.model.model_path);
      }
      for (auto& src_item : src_form.markers) {
         if (src_item.index >= this->_nodes.size())
            continue;
         auto& dst_item = this->_nodes[src_item.index];
         dst_item.entry_points.enabled.overwrite_with_raw_integer(~src_item.disabled_entry_points);
         dst_item.keyword = src_item.keyword.get_form_stub();
      }
      for (size_t i = 0; i < this->_nodes.size(); ++i) {
         if (src_form.active_markers_and_furn_flags & (1 << i))
            this->_nodes[i].enabled = true;
      }
      this->endResetModel();
   }
   void FurnitureMarkersModel::exportData(dovah::loaded_forms::Furniture& dst_form) const {
      size_t size = this->_nodes.size();
      {
         auto& dst_list = dst_form.marker_nif_infos;
         dst_list.clear();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            const auto& src_item = this->_nodes[i];
            auto&       dst_item = dst_list[i];
            dst_item.supported_entry = (uint32_t)src_item.entry_points.supported;
            dst_item.supported_animations |= 1 << ((uint32_t)src_item.animation_type);
         }
      }
      {
         auto& dst_list = dst_form.markers;
         for (auto& dst_item : dst_list)
            dst_item.keyword.set(dst_form, nullptr);
         dst_list.clear();
         for (size_t i = 0; i < size; ++i) {
            const auto& src_item = this->_nodes[i];
            auto&       dst_item = dst_list.emplace_back();
            dst_item.disabled_entry_points = ~(uint32_t)src_item.entry_points.enabled;
            dst_item.index = i;
            dst_item.keyword.set(dst_form, src_item.keyword);
         }
      }
   }

   void FurnitureMarkersModel::setNIF(const std::string& nif_path) {
      auto  nodes_prior = this->_nodes;
      this->_pull_marker_info_from_nif(nif_path);
      auto& nodes_after = this->_nodes;

      bool   reset = false;
      size_t size  = nodes_after.size();
      if (nodes_prior.size() != size)
         reset = true;
      else {
         for (size_t i = 0; i < size; ++i) {
            if (nodes_prior[i].animation_type != nodes_after[i].animation_type) {
               reset = true;
               break;
            }
            if (nodes_prior[i].entry_points.supported != nodes_after[i].entry_points.supported) {
               reset = true;
               break;
            }
         }
      }
      if (reset) {
         this->beginResetModel();
         this->endResetModel();
      } else {
         //
         // Restore states (e.g. what markers and entry points are enabled; what keywords are set).
         //
         nodes_after = std::move(nodes_prior);
      }
   }

   void FurnitureMarkersModel::_pull_marker_info_from_nif(const std::string& nif_path) {
      auto& assets = dovahkit::subsystems::assets::get();
      std::unique_ptr<dovah::bsa_archived_file> file;
      file.reset(assets.lookup_game_asset(std::string("meshes\\") + nif_path));
      if (!file)
         return;

      nifDK::file nif;
      nif.read((void*)file->data(), file->size());
      if (nif.read_error().code != nifDK::default_notice_code) {
         return;
      }

      const auto* block = nif.block_by_name("FRN");
      if (!block)
         return;
      const auto* casted = dynamic_cast<const nifDK::block_types::BSFurnitureMarkerNode*>(block);
      if (!casted)
         return;
      
      auto& src_list = casted->markers;
      auto& dst_list = this->_nodes;
      const size_t size = src_list.size();
      dst_list.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& src_item = src_list[i];
         auto& dst_item = dst_list[i];
         for (size_t i = 0; i < src_item.animation_types.count; ++i) {
            if (src_item.animation_types.test((AnimationType)i)) {
               dst_item.animation_type = (AnimationType)i;
               break;
            }
         }
         dst_item.entry_points.supported = src_item.entry_points;
      }
   }
#pragma endregion

#pragma region FurnitureMarkerEntryPointsProxyModel
   #pragma region QAbstractItemModel /*override*/s
      #pragma region Hierarchy
         /*virtual*/ QModelIndex FurnitureMarkerEntryPointsProxyModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
            if (parent.isValid())
               return {};
            if (column != 0 || row < 0)
               return {};
            if (row >= this->_cache.entry_points.supported_count)
               return {};
            return this->createIndex(row, column, nullptr);
         }
         /*virtual*/ QModelIndex FurnitureMarkerEntryPointsProxyModel::parent(const QModelIndex& index) const /*override*/ {
            return {};
         }
         /*virtual*/ int FurnitureMarkerEntryPointsProxyModel::rowCount(const QModelIndex& parent) const /*override*/ {
            return this->_cache.entry_points.supported_count;
         }
         /*virtual*/ int FurnitureMarkerEntryPointsProxyModel::columnCount(const QModelIndex& parent) const /*override*/ {
            return 1;
         }
      #pragma endregion
      #pragma region Node data
         /*virtual*/ QVariant FurnitureMarkerEntryPointsProxyModel::data(const QModelIndex& index, int role) const /*override*/ {
            if (!index.isValid())
               return {};
            if (index.column() != 0)
               return {};
            if (index.row() >= this->_cache.entry_points.supported_count)
               return {};

            const auto which_opt = _map_row_to_entry_point(index.row());
            if (!which_opt.has_value())
               return {};
            const auto which = which_opt.value();

            switch (role) {
               case Qt::CheckStateRole:
                  return this->_cache.entry_points.enabled.test(which) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
               case Qt::DisplayRole:
                  switch (which) {
                     case EntryPoint::front:
                        return tr("Front");
                     case EntryPoint::back:
                        return tr("Back");
                     case EntryPoint::left:
                        return tr("Left");
                     case EntryPoint::right:
                        return tr("Right");
                     case EntryPoint::up:
                        return tr("Up");
                  }
                  break;
               case KeywordRole:
                  if (auto* model = this->_source_qmi.model())
                     return model->data(this->_source_qmi, KeywordRole);
                  break;
            }
            return {};
         }
         /*virtual*/ Qt::ItemFlags FurnitureMarkerEntryPointsProxyModel::flags(const QModelIndex& index) const /*override*/ {
            return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemFlag::ItemIsUserCheckable;
         }
         #pragma region Write-access
            /*virtual*/ bool FurnitureMarkerEntryPointsProxyModel::setData(const QModelIndex& index, const QVariant& value, int role) /*override*/ {
               if (!index.isValid() || index.model() != this)
                  return false;
               auto* source_model = (FurnitureMarkerEntryPointsProxyModel*) this->_source_qmi.model();
               int   row          = index.row();

               const auto which_opt = _map_row_to_entry_point(index.row());
               if (!which_opt.has_value())
                  return false;
               const auto which = which_opt.value();

               switch (role) {
                  case Qt::CheckStateRole:
                     switch ((Qt::CheckState)value.toInt()) {
                        default:
                           return false;
                        case Qt::CheckState::Checked:
                           this->_cache.entry_points.enabled.set(which);
                        case Qt::CheckState::Unchecked:
                           this->_cache.entry_points.enabled.reset(which);
                           break;
                     }
                     break;
                  case Qt::EditRole:
                     if (value.type() != QVariant::Type::Bool)
                        return false;
                     if (value.toBool())
                        this->_cache.entry_points.enabled.set(which);
                     else
                        this->_cache.entry_points.enabled.reset(which);
                     role = Qt::CheckStateRole;
                     break;
                  case KeywordRole:
                     if (!source_model)
                        return false;
                     return source_model->setData(this->_source_qmi, value, role);
                  default:
                     return false;
               }
               emit dataChanged(index, index, { role });
               return true;
            }
         #pragma endregion
      #pragma endregion
      /*virtual*/ QVariant FurnitureMarkerEntryPointsProxyModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (section != 0)
            return {};
         if (role != Qt::DisplayRole)
            return {};
         return tr("Entry Point");
      }
   #pragma endregion

   QModelIndex FurnitureMarkerEntryPointsProxyModel::source() const {
      return this->_source_qmi;
   }
   void FurnitureMarkerEntryPointsProxyModel::setSource(const QModelIndex& qmi) {
      if (this->_source_qmi == qmi)
         return;
      this->beginResetModel();
      if (auto* prior_model = this->_source_qmi.model()) {
         QObject::disconnect(prior_model, nullptr, this, nullptr);
      }
      this->_source_qmi = qmi;
      if (qmi.isValid()) {
         auto* model = qmi.model();
         QObject::connect(model, &QAbstractItemModel::dataChanged, this, &FurnitureMarkerEntryPointsProxyModel::_recache);
      }
      this->_recache();
      this->endResetModel();
   }

   std::optional<FurnitureMarkerEntryPointsProxyModel::EntryPoint> FurnitureMarkerEntryPointsProxyModel::_map_row_to_entry_point(int row) const {
      if (row < 0)
         return {};
      int which = 0;
      for (size_t i = 0; i < valid_entry_point_count; ++i) {
         if (!this->_cache.entry_points.supported.test((EntryPoint)i))
            continue;
         ++which;
         if (which == row)
            return (EntryPoint)which;
      }
      return {};
   }
   void FurnitureMarkerEntryPointsProxyModel::_recache() {
      if (!this->_source_qmi.isValid()) {
         this->beginRemoveRows({}, 0, this->rowCount());
         this->_cache = {};
         this->endRemoveRows();
         return;
      }
      auto* source_model = this->_source_qmi.model();

      size_t row_count_prior = this->_cache.entry_points.supported_count;
      
      this->_cache.entry_points.supported.overwrite_with_raw_integer((uint32_t)source_model->data(this->_source_qmi, source_model_type::EntryPointsSupportedRole).toInt());
      this->_cache.entry_points.supported_count = this->_cache.entry_points.supported.number_set();
      this->_cache.entry_points.enabled.overwrite_with_raw_integer((uint32_t)source_model->data(this->_source_qmi, source_model_type::EntryPointsEnabledRole).toInt());

      size_t row_count_after = this->_cache.entry_points.supported_count;

      if (row_count_after > 0) {
         emit dataChanged(
            this->index(0, 0, {}),
            this->index(row_count_after - 1, 0, {})
         );
      }
      if (row_count_prior > row_count_after) {
         this->beginRemoveRows({}, row_count_after, row_count_prior - 1);
         this->endRemoveRows();
      } else if (row_count_prior < row_count_after) {
         this->beginInsertRows({}, row_count_prior, row_count_after - 1);
         this->endInsertRows();
      }
   }
#pragma endregion