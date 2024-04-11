#pragma once
#include <cstdint>
#include <optional>
#include <QPushButton>
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/forms/components/destruction.h"
   #include "ui/types/nif_for_form.h"
#endif

namespace dovah {
   class form_stub;
}

class DKFormDestructionDataButton : public QWidget {
   Q_OBJECT;
   public:
      #if !defined(QT_DESIGNER_LIB)
         using form_data_type = dovah::loaded_forms::components::destruction_stage_data;

         enum class DestructionStageFlag {
            cap_damage     = 0x01, // "When the object is at this stage, prevents you from skipping multiple stages by inflicting massive damage."
            disable_object = 0x02,
            destroy_object = 0x04,
            ignore_external_damage = 0x08, // "Prevents the object from taking damage from anything other than Self Damage Per Second."
         };
         Q_DECLARE_FLAGS(DestructionStageFlags, DestructionStageFlag);
         
         struct DestructionStage {
            uint16_t                health_percent   = 100;
            uint8_t                 damage_stage     = 0;
            DestructionStageFlags   flags            = {};
            uint32_t                self_damage_rate = 0; // the object inflicts this much damage on itself per second (i.e. health decay)
            dovah::form_stub*       explosion        = nullptr;
            dovah::form_stub*       debris           = nullptr;
            uint32_t                debris_count     = 0;
            ui::types::nif_for_form replacement_model;
         };

         struct DestructionData {
            uint32_t health = 1;
            struct {
               bool vats_enabled = false;
            } flags;
            std::vector<DestructionStage> stages;
         };
      #endif

   public:
      DKFormDestructionDataButton(QWidget* parent = nullptr);
      
      #if !defined(QT_DESIGNER_LIB)
         void initializeFrom(const std::optional<form_data_type>&);
         void commitTo(std::optional<form_data_type>&, dovah::loaded_forms::Form& owner);
      #endif

   protected:
      QPushButton* _button = nullptr;
      #if !defined(QT_DESIGNER_LIB)
         std::optional<DestructionData> _value;
      #endif
};

#if !defined(QT_DESIGNER_LIB)
   Q_DECLARE_OPERATORS_FOR_FLAGS(DKFormDestructionDataButton::DestructionStageFlags);
#endif