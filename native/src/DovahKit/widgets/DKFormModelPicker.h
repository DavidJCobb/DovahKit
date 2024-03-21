#pragma once
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/forms/structs/precached_nif_info.h"
#endif

#if !defined(QT_DESIGNER_LIB)
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

class DKFormModelPicker : QWidget {
   Q_OBJECT;
   public:
      struct TextureSwap {
         std::string block_name;
         size_t      block_index = 0;
         dovah::form_stub* texture_set = nullptr;
      };

   public:
      DKFormModelPicker(QWidget* parent = nullptr);

      #if !defined(QT_DESIGNER_LIB)
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
      struct {
         std::string model_path;
         dovah::loaded_forms::precached_nif_info precached_nif_info;
         bool supports_texture_swaps = false;
         std::vector<TextureSwap> texture_swaps;
      } _state;
};