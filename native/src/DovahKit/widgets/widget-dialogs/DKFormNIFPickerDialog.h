#pragma once
#include "ui_DKFormNIFPickerDialog.h" // generated
#include <QDialog>
#include "dovah/forms/structs/precached_nif_info.h"
#include "ui/types/nif_texture_swap.h"

#include "../DKFormNIFPicker.h"

namespace nifDK {
   class file;
}

class DKFormNIFPickerDialogTextureSwapModel;

class DKFormNIFPickerDialog : public QDialog {
   Q_OBJECT;
   protected:
      using PrecachedNIFInfo = dovah::loaded_forms::precached_nif_info;

   public:
      DKFormNIFPickerDialog(QWidget* parent = nullptr);

      QString modelPath() const;

      constexpr bool textureSwapsAllowed() const noexcept { return this->_state.supports_texture_swaps; }

      constexpr const std::vector<ui::types::nif_texture_swap>& textureSwaps() const { return this->_state.texture_swaps; }

      constexpr const PrecachedNIFInfo& precachedNIFInfo() const { return this->_state.precached_nif_info; }

   public slots:
      void setModelPath(QString);
      void setTextureSwaps(const std::vector<ui::types::nif_texture_swap>&);
      void setTextureSwapsAllowed(bool);

   protected:
      Ui::DKFormNIFPickerDialog ui;
      struct {
         PrecachedNIFInfo precached_nif_info;
         bool supports_texture_swaps = false;

         std::vector<ui::types::nif_texture_swap> texture_swaps;
         size_t end_of_nif_defined_swaps = 0; // all swaps at and past this index come from the user, not the NIF
      } _state;
      DKFormNIFPickerDialogTextureSwapModel* _model = nullptr;

      void _reload_all_nif_info();

      void _reload_texture_swap_blocks(const nifDK::file&);

      void _refresh_texture_swaps();

      void _set_texture_set(size_t row, dovah::form_stub*);
};
