#pragma once
#include "ui_DKFormModelPickerDialog.h" // generated
#include <QDialog>
#include "dovah/forms/structs/precached_nif_info.h"

#include "../DKFormModelPicker.h"

namespace nifDK {
   class file;
}

class DKFormModelPickerDialog : public QDialog {
   Q_OBJECT;
   protected:
      using PrecachedNIFInfo = dovah::loaded_forms::precached_nif_info;
      using TextureSwap      = DKFormModelPicker::TextureSwap;

   public:
      DKFormModelPickerDialog(QWidget* parent = nullptr);

      QString modelPath() const;

      constexpr bool textureSwapsAllowed() const noexcept { return this->_state.supports_texture_swaps; }

      constexpr const std::vector<TextureSwap>& textureSwaps() const { return this->_state.texture_swaps; }

      constexpr const PrecachedNIFInfo& precachedNIFInfo() const { return this->_state.precached_nif_info; }

   public slots:
      void setModelPath(QString);
      void setTextureSwaps(const std::vector<TextureSwap>&);
      void setTextureSwapsAllowed(bool);

      virtual bool eventFilter(QObject* watched, QEvent*);

   protected:
      Ui::DKFormModelPickerDialog ui;
      struct {
         PrecachedNIFInfo precached_nif_info;
         bool supports_texture_swaps = false;
         std::vector<TextureSwap> texture_swaps;
      } _state;

      void _reload_all_nif_info();

      void _reload_texture_swap_blocks(const nifDK::file&);

      void _refresh_texture_swaps();

      void _set_texture_set(size_t row, dovah::form_stub*);
};
