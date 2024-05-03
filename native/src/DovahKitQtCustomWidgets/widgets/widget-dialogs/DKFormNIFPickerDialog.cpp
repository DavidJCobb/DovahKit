#include "./DKFormNIFPickerDialog.h"
#include <QDropEvent>
#include <QMimeData>

#include "dovah/files/bsa/bsa_archived_file.h"
#include "editor/helpers/form_stub_drag_drop.h"
#include "editor/subsystems/assets.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"

#include "nif/blocks/BSLightingShaderProperty.h"
#include "nif/blocks/BSShaderTextureSet.h"
#include "nif/blocks/NiGeometry.h"
#include "nif/utils/find_all_retexturable_blocks.h"
#include "nif/utils/precache_nif_info.h"
#include "nif/file.h"

#include "../DKFormPickerDialog.h"
#include "../DKHeaderView.h"

#pragma region DKFormNIFPickerDialogTextureSwapModel
#include "ui/models/DKGenericListModel.h"

struct _model_node_type : public ui::types::nif_texture_swap {
   bool is_in_nif = false;
};

class DKFormNIFPickerDialogTextureSwapModel : public DKGenericListModel<DKFormNIFPickerDialogTextureSwapModel, ui::types::nif_texture_swap> {
   public:
      struct Column {
         Column() = delete;
         enum : size_t {
            BlockName,
            LeafIndex,
            TextureSet,
         };
      };
      static constexpr const size_t column_count = 3; // override

   public:
      DKFormNIFPickerDialogTextureSwapModel(QObject* parent) : DKGenericListModel(parent) {}

      using DKGenericListModel::clear;

      #pragma region Overrides
         QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
            switch (role) {
               case Qt::UserRole:
                  return QVariant::fromValue(node.texture_set);

               case Qt::TextAlignmentRole:
                  if (column == Column::LeafIndex) {
                     return (int)(Qt::AlignRight | Qt::AlignBaseline);
                  }
                  return {};

               case Qt::DisplayRole:
               case Qt::ToolTipRole:
                  switch (column) {
                     case Column::BlockName:
                        return QString::fromStdString(node.block_name);
                     case Column::LeafIndex:
                        return node.leaf_index;
                     case Column::TextureSet:
                        if (node.texture_set == nullptr)
                           return {};
                        return QString::fromStdString(node.texture_set->editorID);
                  }
                  return {};
            }
            return {};
         }
         Qt::ItemFlags flags_of(const node_type&, size_t column) const {
            return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         }

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
            if (role != Qt::DisplayRole)
               return {};
            if (orientation != Qt::Orientation::Horizontal)
               return {};
            switch (section) {
               case Column::BlockName:
                  return tr("3D Name");
               case Column::LeafIndex:
                  return tr("3D Index");
               case Column::TextureSet:
                  return tr("New Texture");
            }
            return {};
         }
      #pragma endregion

      void replaceItems(const std::vector<node_type>& items) {
         this->beginResetModel();

         this->_nodes.clear();
         for (auto& item : items) {
            auto* node = new node_type{item};
            this->_nodes.push_back(node);
         }

         this->endResetModel();
      }

      void setTextureSet(int row, dovah::form_stub* form) {
         if (row < 0 || row >= this->_nodes.size())
            return;
         auto* node = this->_nodes[row];
         if (!node)
            return;
         if (node->texture_set == form)
            return;

         node->texture_set = form;
         this->emitNodeChanged(*node, Column::TextureSet);
      }
};
#pragma endregion

DKFormNIFPickerDialog::DKFormNIFPickerDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   {
      auto* view = this->ui.textureSwaps;

      this->_model = new DKFormNIFPickerDialogTextureSwapModel(this);
      view->setModel(this->_model);

      if (auto* h = view->verticalHeader()) {
         h->setVisible(false);
         h->setSectionResizeMode(QHeaderView::ResizeToContents);
      }
      {
         auto* header = new DKHeaderView(Qt::Horizontal, view);
         header->setFlexResizeEnabled(true);
         view->setHorizontalHeader(header);
         //
         auto metrics = QFontMetrics(view->font());
         header->setMinimumSectionSize(2);
         header->setColumnFlex(0, 1, 0);
         header->setColumnFlex(1, 0, 0, metrics.boundingRect("000").width() * 1.5F + 4); // size of NIF block index column
      }

      QObject::connect(view, &QTableView::doubleClicked, this, [this](const QModelIndex& index) {
         auto row = index.row();
         if (row < 0)
            return;

         auto* dialog = new DKFormPickerDialog(this);
         dialog->setAllowedFormType(dovah::form_type::texture_set);

         // We want this to block all the way up to the form-editing dialog, but not further.
         dialog->setWindowModality(Qt::WindowModal);

         QObject::connect(dialog, &QDialog::accepted, this, [this, dialog, row]() {
            auto* stub = dialog->formStub();
            this->_set_texture_set(row, stub);
         });
         QObject::connect(dialog, &QDialog::finished, dialog, [dialog]() {
            dialog->deleteLater();
         });

         dialog->show();
      });
   }

   this->ui.filePicker->setStandardConfiguration(DKGameFilePicker::StandardConfiguration::Meshes);
   this->ui.filePicker->setPathFormat(DKGameFilePicker::PathFormat::OmitPathStem);
   QObject::connect(this->ui.filePicker, &DKGameFilePicker::pathChanged, this, [this]() {
      this->_reload_all_nif_info();
      this->_refresh_texture_swaps();
   });

   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, [this]() {
      this->accept();
   });
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, [this]() {
      this->reject();
   });
}

QString DKFormNIFPickerDialog::modelPath() const {
   return this->ui.filePicker->rawPath();
}
void DKFormNIFPickerDialog::setModelPath(QString path) {
   this->ui.filePicker->setRawPath(path);
}

void DKFormNIFPickerDialog::setTextureSwaps(const std::vector<ui::types::nif_texture_swap>& swaps) {
   //
   // We need to coalesce these sets. The general flow is that you set the model path, causing us 
   // to look up the NIF and precache its remappable-texture geometry blocks; and then you set any 
   // pre-existing texture swaps, so we want to integrate those into the list of blocks we found 
   // rather than bulldozing them. (If you haven't swapped any textures *yet*, then operator= would 
   // cause us to lose all the blocks!)
   //
   for (auto& dst : this->_state.texture_swaps) {
      dst.texture_set = nullptr;
   }
   for (const auto& src : swaps) {
      bool found = false;
      bool maybe = false;
      for (auto& dst : this->_state.texture_swaps) {
         bool index = dst.leaf_index == src.leaf_index;
         bool name  = dst.block_name == src.block_name;
         if (index || name) {
            maybe = true;
            if (index && name) {
               dst.texture_set = src.texture_set;
               found = true;
               break;
            }
         }
      }
      if (!found && !maybe) {
         //
         // Current handling is, if the form has a texture swap defined that isn't present on the 
         // current model, then we should append it to the list. We define "not being present" as 
         // a texture swap whose index AND name don't match anything on the current model.
         // 
         // TODO: How does the game itself map texture swaps to an in-memory NIF? Does it rely on 
         //       both the block index and block name, or does it only bother with the index?  We 
         //       should mimic that behavior here.
         //
         this->_state.texture_swaps.push_back(src);
      }
   }

   this->_refresh_texture_swaps();
}
void DKFormNIFPickerDialog::setTextureSwapsAllowed(bool allowed) {
   if (this->_state.supports_texture_swaps == allowed)
      return;
   if (!allowed)
      this->_state.texture_swaps = {};
   this->_state.supports_texture_swaps = allowed;
   this->_refresh_texture_swaps();
}

void DKFormNIFPickerDialog::_reload_all_nif_info() {
   this->_state.precached_nif_info = {};
   this->_state.texture_swaps      = {};

   auto  path   = std::string("meshes/") + this->modelPath().toStdString();
   auto& assets = dovahkit::subsystems::assets::get_or_create();
   auto* loaded = assets.lookup_game_asset(path, true);
   if (loaded) {
      nifDK::file file;
      file.read((void*)loaded->data(), loaded->size());
      if (file.read_error().empty()) {
         this->_state.precached_nif_info = nifDK::utils::precache_nif_info(file);
         this->_reload_texture_swap_blocks(file);
      }
   }
}

void DKFormNIFPickerDialog::_reload_texture_swap_blocks(const nifDK::file& file) {
   this->_state.texture_swaps.clear();

   auto list = nifDK::utils::find_all_retexturable_blocks(file);
   for (auto& item : list) {
      this->_state.texture_swaps.push_back({
         .block_name = item.second,
         .leaf_index = item.first,
      });
   }
}

void DKFormNIFPickerDialog::_refresh_texture_swaps() {
   auto* table = this->ui.textureSwaps;

   table->setEnabled(this->textureSwapsAllowed());
   if (this->textureSwapsAllowed()) {
      this->_model->replaceItems(this->_state.texture_swaps);
   } else {
      this->_model->replaceItems({});
   }
}

void DKFormNIFPickerDialog::_set_texture_set(size_t row, dovah::form_stub* stub) {
   if (!this->textureSwapsAllowed())
      return;
   
   if (row >= this->_state.texture_swaps.size())
      return;

   this->_state.texture_swaps[row].texture_set = stub;
   this->_model->setTextureSet(row, stub);
}