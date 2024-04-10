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
#include "nif/utils/precache_nif_info.h"
#include "nif/file.h"

#include "../DKFormPickerDialog.h"
#include "../DKHeaderView.h"

DKFormNIFPickerDialog::DKFormNIFPickerDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   if (auto* h = this->ui.textureSwaps->verticalHeader()) {
      h->setVisible(false);
      h->setSectionResizeMode(QHeaderView::ResizeToContents);
   }
   {
      auto* header = new DKHeaderView(Qt::Horizontal, this->ui.textureSwaps);
      header->setFlexResizeEnabled(true);
      this->ui.textureSwaps->setHorizontalHeader(header);
      //
      auto metrics = QFontMetrics(this->ui.textureSwaps->font());
      header->setMinimumSectionSize(2);
      header->setColumnFlex(0, 1, 0);
      header->setColumnFlex(1, 0, 0, metrics.boundingRect("000").width() * 1.5F + 4); // size of NIF block index column
   }

   this->ui.filePicker->setStandardConfiguration(DKGameFilePicker::StandardConfiguration::Meshes);
   this->ui.filePicker->setPathFormat(DKGameFilePicker::PathFormat::OmitPathStem);
   QObject::connect(this->ui.filePicker, &DKGameFilePicker::pathChanged, this, [this]() {
      this->_reload_all_nif_info();
      this->_refresh_texture_swaps();
   });

   QObject::connect(this->ui.textureSwaps, &QTableWidget::cellDoubleClicked, this, [this](int row, int column) {
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

   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, [this]() {
      this->accept();
   });
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, [this]() {
      this->reject();
   });
}

QString DKFormNIFPickerDialog::modelPath() const {
   return this->ui.filePicker->path();
}
void DKFormNIFPickerDialog::setModelPath(QString path) {
   this->ui.filePicker->setPath(path);
}

void DKFormNIFPickerDialog::setTextureSwaps(const std::vector<ui::types::nif_texture_swap>& swaps) {
   /*
   this->_state.texture_swaps = swaps;
   */

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
         bool index = dst.block_index == src.block_index;
         bool name  = dst.block_name  == src.block_name;
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

   for (size_t block_index = 0; block_index < file.all_blocks.size(); ++block_index) {
      const auto* block = file.all_blocks[block_index];
      if (!block)
         continue;
      auto* geometry = dynamic_cast<const nifDK::block_types::NiGeometry*>(block);
      if (!geometry)
         continue;
      if (!geometry->properties.shader)
         continue;
      auto* shader = dynamic_cast<const nifDK::block_types::BSLightingShaderProperty*>(geometry->properties.shader);
      if (!shader)
         continue;
      if (!(shader->shader_flags[0] & nifDK::SkyrimShaderPropertyFlagA::remappable_textures))
         continue;
      this->_state.texture_swaps.push_back({
         .block_name  = geometry->name,
         .block_index = block_index,
      });
   }
}

void DKFormNIFPickerDialog::_refresh_texture_swaps() {
   auto* table = this->ui.textureSwaps;

   table->setEnabled(this->textureSwapsAllowed());
   table->setRowCount(0);
   
   if (this->textureSwapsAllowed()) {
      const auto& src = this->_state.texture_swaps;
      table->setRowCount(src.size());

      for (size_t i = 0; i < src.size(); ++i) {
         auto& src_item = src[i];

         QTableWidgetItem* cell = nullptr;

         cell = table->item(i, 0);
         if (!cell) {
            cell = new QTableWidgetItem;
            table->setItem(i, 0, cell);
         }
         cell->setText(QString::fromStdString(src_item.block_name));

         cell = table->item(i, 1);
         if (!cell) {
            cell = new QTableWidgetItem;
            table->setItem(i, 1, cell);
         }
         cell->setText(QString::number(src_item.block_index));

         cell = table->item(i, 2);
         if (!cell) {
            cell = new QTableWidgetItem;
            table->setItem(i, 2, cell);
         }
         auto data = QVariant::fromValue(src_item.texture_set);
         cell->setText(data.toString());
         cell->setData(Qt::UserRole, data);
      }
   }
}

void DKFormNIFPickerDialog::_set_texture_set(size_t row, dovah::form_stub* stub) {
   if (!this->textureSwapsAllowed())
      return;

   auto* table = this->ui.textureSwaps;
   auto* cell  = table->item(row, 2);
   if (!cell)
      return;
   if (stub) {
      auto data = QVariant::fromValue(stub);
      cell->setText(data.toString());
      cell->setData(Qt::UserRole, data);
   } else {
      auto data = QVariant::fromValue(stub);
      cell->setText("");
      cell->setData(Qt::UserRole, data);
   }
}