#pragma once
#include <cstdint>
#include <QComboBox>

class DKNavmeshGenerationImportOptionPicker : public QWidget {
   Q_OBJECT;
   public:
      enum class Value : uint32_t {
         //
         // All forms that have a Navmesh Generation Import Option use the same set of 
         // flags for it, including the absence of any bits set to indicate "Collision 
         // Geometry." This is confirmed via reverse-engineering of the 32-bit Creation 
         // Kit: the GUI code for TESForm blindly sets the relevant radio buttons (if
         // they exist) based on those flags, regardless of form type.
         // 
         // Of course, forms that *don't* have a Navmesh Generation Import Option may 
         // use these record flags for any other purpose.
         //
         CollisionGeometry = 0x00000000,
         BoundingBox       = 0x08000000,
         Filter            = 0x04000000,
         Ground            = 0x40000000,
      };

   public:
      DKNavmeshGenerationImportOptionPicker(QWidget* parent = nullptr);

      Value getValue() const;
      void setValue(Value);

      void setValueByMask(uint32_t);
      void writeValueToMask(uint32_t& dst) const;

   signals:
      void valueChanged(Value);

   protected:
      struct {
         QComboBox* combobox = nullptr;
      } _subwidgets;
};