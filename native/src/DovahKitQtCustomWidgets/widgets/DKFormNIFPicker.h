#pragma once
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#if !defined(QT_PLUGIN)
   #include "dovah/forms/structs/precached_nif_info.h"
   #include "ui/types/nif_texture_swap.h"
#endif

#if !defined(QT_PLUGIN)
namespace dovah {
   namespace loaded_forms {
      namespace components {
         class model;
      }
      class Form;
   }
}
#endif

namespace dovah {
   class form_stub;
}

class DKFormNIFPicker : public QWidget {
   Q_OBJECT;
   public:
      DKFormNIFPicker(QWidget* parent = nullptr);

      #if !defined(QT_PLUGIN)
         void initializeFrom(const dovah::loaded_forms::components::model&);
         void commitTo(dovah::loaded_forms::components::model&, dovah::loaded_forms::Form& owner);
      #endif

   signals:
      void dataChanged();

   protected:
      struct {
         QLineEdit*   path   = nullptr;
         QPushButton* button = nullptr;
      } _subwidgets;
      #if !defined(QT_PLUGIN)
         struct {
            std::string model_path;
            dovah::loaded_forms::precached_nif_info precached_nif_info;
            bool supports_texture_swaps = false;
            std::vector<ui::types::nif_texture_swap> texture_swaps;
         } _state;
      #endif
};