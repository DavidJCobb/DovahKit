#include "./bind.h"
#include <cassert>
#include <QCheckBox>
#include <QLineEdit>
#include "dovah/forms/structs/color_dword.h"
#include "dovah/forms/Form.h" // for working copies
#include "dovah/forms/Sound.h" // for mapping TESSound to BGSSoundDescriptor.
#include "dovah/form_reference_t.h"
#include "editor/asset_manager/asset_manager.h" // for DKTextureAssetPane and whatnot
#include "ui/types/game_file_path.h"
#include "widgets/DKColorPickerButton.h"
#include "widgets/DKCompactObjectReferencePicker.h"
#include "widgets/DKFloatSlider.h"
#include "widgets/DKFormPicker.h"
#include "widgets/widget-data/DKCustomFormFilter.h"
#include "widgets/DKFormListPane.h"
#include "widgets/DKGameFilePicker.h"
#include "widgets/DKNavmeshGenerationImportOptionPicker.h"
#include "widgets/DKObjectReferencePicker.h"
#include "widgets/DKTextureAssetPane.h"

namespace ui {
   extern void bind(QCheckBox* widget, bool& target) {
      widget->setChecked(target);
      QObject::connect(widget, &QCheckBox::stateChanged, widget, [&target](int state) {
         target = state == Qt::CheckState::Checked;
      });
   }
   extern void bind(QGroupBox* widget, bool& target) {
      widget->setChecked(target);
      QObject::connect(widget, &QGroupBox::toggled, widget, [&target](bool checked) {
         target = checked;
      });
   }
   extern void bind_inverse(QCheckBox* widget, bool& target) {
      widget->setChecked(!target);
      QObject::connect(widget, &QCheckBox::stateChanged, widget, [&target](int state) {
         target = state != Qt::CheckState::Checked;
      });
   }
   extern void bind_inverse(QGroupBox* widget, bool& target) {
      widget->setChecked(!target);
      QObject::connect(widget, &QGroupBox::toggled, widget, [&target](bool checked) {
         target = !checked;
      });
   }

   extern void bind(QLineEdit* widget, std::string& target) {
      widget->setText(QString::fromUtf8(QByteArray::fromStdString(target)));
      QObject::connect(widget, &QLineEdit::textChanged, widget, [&target](const QString& value) {
         target = value.toUtf8().toStdString();
      });
   }

   extern void bind(DKColorPickerButton* widget, dovah::loaded_forms::color_t& target) {
      widget->setColor(QColor::fromRgb(
         target.r,
         target.g,
         target.b
      ));
      QObject::connect(widget, &DKColorPickerButton::colorChanged, widget, [widget, &target](QColor color) {
         target.r = color.red();
         target.g = color.green();
         target.b = color.blue();
         if (widget->hasAlpha()) {
            target.unused = color.alpha();
         }
      });
   }

   extern void bind(DKFormPicker* widget, dovah::form_stub*& target) {
      widget->setFormStub(target);
      QObject::connect(widget, &DKFormPicker::formChanged, widget, [&target](dovah::form_stub* value) {
         target = value;
      });
      //
      // Account for the possibility that the widget may have rejected the 
      // pre-existing value. (We could do this by just hooking our signal 
      // handler before we set the stub, but then we'd catch a spurious 
      // signal for setting the initial value.)
      //
      if (auto* stub = target) {
         bool changed = false;
         if (auto* filter = widget->customFilter()) {
            if (!filter->form_matches(*stub)) {
               changed = true;
            }
         }
         if (!changed) {
            auto& list    = widget->allowedFormTypes();
            bool  allowed = list.empty();
            for (auto ft : list) {
               if (target->form_type == ft) {
                  allowed = true;
                  break;
               }
            }
            if (!allowed)
               changed = true;
         }

         if (changed) {
            target = widget->formStub();
         }
      } else if (!widget->allowNone()) {
         if (auto* after = widget->formStub())
            target = after;
      }
   }

   extern void bind(DKFloatSlider* widget, float& dst) {
      widget->setValue(dst);
      QObject::connect(widget, &DKFloatSlider::valueChanged, widget, [&dst](float value) {
         dst = value;
      });
   }

   extern void bind(DKFormPicker* widget, dovah::form_reference_t& dst, dovah::loaded_forms::Form& dst_owner) {
      assert(dst_owner.is_working_copy && "This function was created to make things easier for the (messy) form-working-copy system. Don't use it for real forms.");
      auto* stub = dst.get_form_stub();
      if (stub && stub->form_type == dovah::form_type::sound) {
         if (!widget->allowsFormType(dovah::form_type::sound) && widget->allowsFormType(dovah::form_type::sound_descriptor)) {
            //
            // HACK to match Skyrim and CK behavior. Bethesda moved a lot of 
            // data from SOUN to SNDR, and modified a lot of loading code to 
            // silently replace SOUN references with references to the SNDR 
            // that the SOUN wraps. We don't do that on load, but we'll do 
            // it here to ensure that the UI doesn't get cleared out.
            //
            auto loaded = stub->load().ptr_cast<dovah::loaded_forms::Sound>();
            if (loaded && loaded->descriptor)
               stub = loaded->descriptor.get_form_stub();
         }
      }
      widget->setFormStub(stub);
      QObject::connect(widget, &DKFormPicker::formChanged, widget, [&dst, &dst_owner](dovah::form_stub* value) {
         dst.set(dst_owner, value);
      });
      //
      // Account for the possibility that the widget may have rejected the 
      // pre-existing value. (We could do this by just hooking our signal 
      // handler before we set the stub, but then we'd catch a spurious 
      // signal for setting the initial value.)
      //
      if (stub) {
         if (auto* filter = widget->customFilter()) {
            if (!filter->form_matches(*stub)) {
               dst.set(dst_owner, widget->formStub());
            }
         }
      } else if (!widget->allowNone()) {
         if (auto* after = widget->formStub())
            dst.set(dst_owner, after);
      }
   }
   extern void bind(DKCompactObjectReferencePicker* widget, dovah::form_reference_t& dst, dovah::loaded_forms::Form& dst_owner) {
      assert(dst_owner.is_working_copy && "This function was created to make things easier for the (messy) form-working-copy system. Don't use it for real forms.");
      widget->setRef(dst.get_form_stub());
      QObject::connect(widget, &DKCompactObjectReferencePicker::refChanged, widget, [&dst, &dst_owner](dovah::form_stub* value) {
         dst.set(dst_owner, value);
      });
   }
   extern void bind(DKObjectReferencePicker* widget, dovah::form_reference_t& dst, dovah::loaded_forms::Form& dst_owner) {
      assert(dst_owner.is_working_copy && "This function was created to make things easier for the (messy) form-working-copy system. Don't use it for real forms.");
      widget->setRef(dst.get_form_stub());
      QObject::connect(widget, &DKObjectReferencePicker::refChanged, widget, [&dst, &dst_owner](dovah::form_stub* value) {
         dst.set(dst_owner, value);
      });
   }

   extern void bind(DKGameFilePicker* widget, std::string& dst) {
      auto stem = widget->pathStem();
      if (!stem.empty()) {
         widget->setValue(stem.append(QString::fromStdString(dst)));
      } else {
         widget->setValue(ui::types::game_file_path(QString::fromStdString(dst)));
      }
      QObject::connect(widget, &DKGameFilePicker::valueChanged, widget, [widget, &dst](ui::types::game_file_path path) {
         {
            auto stem = widget->pathStem();
            if (!stem.empty())
               path = path.lexically_relative(stem);
         }
         dst = path.to_string().toStdString();
      });
   }

   extern void bind(DKNavmeshGenerationImportOptionPicker* widget, uint32_t& record_flags) {
      widget->setValueByMask(record_flags);
      QObject::connect(widget, &DKNavmeshGenerationImportOptionPicker::valueChanged, widget, [widget, &record_flags](auto value) {
         widget->writeValueToMask(record_flags);
      });
   }

   extern void bind(DKGameFilePicker* picker, DKTextureAssetPane* preview) {
      QObject::connect(picker, &DKGameFilePicker::valueChanged, preview, [picker, preview](const ui::types::game_file_path& path) {
         if (path.empty()) {
            preview->setAsset(nullptr);
            return;
         }
         preview->setAsset(DovahKitAssetManager::get().requestAsset(path.lexically_relative("Data\\").to_string()));
      });
   }
}