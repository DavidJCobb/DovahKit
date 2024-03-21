#include "./DKFormModelPickerDialog.h"
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

DKFormModelPickerDialog::DKFormModelPickerDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   this->setWindowTitle(tr("Select Form"));

   this->ui.textureSwaps->installEventFilter(this);

   this->ui.filePicker->setStandardConfiguration(DKGameFilePicker::StandardConfiguration::Meshes);
   QObject::connect(this->ui.filePicker, &DKGameFilePicker::pathChanged, this, [this]() {
      this->ui.textureSwaps = {};
      this->_reload_all_nif_info();
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

QString DKFormModelPickerDialog::modelPath() const {
   return this->ui.filePicker->path();
}
void DKFormModelPickerDialog::setModelPath(QString path) {
   this->ui.filePicker->setPath(path);
}

void DKFormModelPickerDialog::setTextureSwaps(const std::vector<TextureSwap>& swaps) {
   this->_state.texture_swaps = swaps;
   this->_refresh_texture_swaps();
}
void DKFormModelPickerDialog::setTextureSwapsAllowed(bool allowed) {
   if (this->_state.supports_texture_swaps == allowed)
      return;
   if (!allowed)
      this->_state.texture_swaps = {};
   this->_state.supports_texture_swaps = allowed;
   this->_refresh_texture_swaps();
}

void DKFormModelPickerDialog::_reload_all_nif_info() {
   this->_state.precached_nif_info = {};
   this->_state.texture_swaps      = {};

   auto  path   = this->modelPath().toStdString();
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

void DKFormModelPickerDialog::_reload_texture_swap_blocks(const nifDK::file& file) {
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
      this->_state.texture_swaps.emplace_back(TextureSwap{
         .block_name  = geometry->name,
         .block_index = block_index,
      });
   }
}

void DKFormModelPickerDialog::_refresh_texture_swaps() {
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

void DKFormModelPickerDialog::_set_texture_set(size_t row, dovah::form_stub* stub) {
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

/*virtual*/ bool DKFormModelPickerDialog::eventFilter(QObject* watched, QEvent* untyped_event) /*override*/ {
   if (untyped_event->type() != QEvent::Type::Drop)
      return false;
   if (watched != this->ui.textureSwaps)
      return false;

   auto* event = (QDropEvent*)untyped_event;
   if (event->source() != watched)
      return false;
   if (event->dropAction() != Qt::DropAction::CopyAction)
      return false;
   if (event->isAccepted()) // data already moved?
      return false;

   const auto* mime_data = event->mimeData();
   if (!mime_data->hasFormat(editor_helpers::single_form_stub_mime_type))
      return false;

   dovah::form_stub* dropped_form = editor_helpers::single_form_stub_from_mime_data(*mime_data);
   if (!dropped_form)
      return false;
   if (dropped_form->form_type != dovah::form_type::texture_set) {
      event->setDropAction(Qt::DropAction::IgnoreAction);
      event->accept();
      return true;
   }

   auto* widget = this->ui.textureSwaps;

   auto viewpoint_pos = widget->viewport()->mapFromGlobal(event->pos());
   auto qmi = widget->indexAt(viewpoint_pos);
   if (!qmi.isValid())
      return false;

   this->_set_texture_set(qmi.row(), dropped_form);
   event->acceptProposedAction();
   event->accept();
   return true;
}