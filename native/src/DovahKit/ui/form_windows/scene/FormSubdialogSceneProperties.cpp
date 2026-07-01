#include "./FormSubdialogSceneProperties.h"
#include <cassert>
#include "dovah/forms/components/papyrus/fragment_data/scene_fragment_data.h"
#include "dovah/forms/Scene.h"

FormSubdialogSceneProperties::FormSubdialogSceneProperties(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);
   this->setWindowFlags(this->windowFlags() | Qt::WindowContextHelpButtonHint); // show "What's This?" button in title bar
}

void FormSubdialogSceneProperties::importData(loaded_form_type& src_form) {
   #pragma region Papyrus
      this->ui.scriptListPane->setFormWorkingCopy(&src_form); // handle this widget first since the fragment pickers use it as a data source
      {
         auto* fragdata = (papyrus_fragment_data_type*)src_form.script_data.fragment_data;
         if (fragdata) {
            assert(fragdata->type == dovah::loaded_forms::components::papyrus::fragment_type::scene);
            if (auto& opt = fragdata->fragments.on_begin; opt.has_value()) {
               auto& src = opt.value();
               auto* dst = this->ui.fragmentBegin;
               dst->setCurrentScriptname(src.script);
               dst->setCurrentFunction(src.function);
            }
            if (auto& opt = fragdata->fragments.on_end; opt.has_value()) {
               auto& src = opt.value();
               auto* dst = this->ui.fragmentEnd;
               dst->setCurrentScriptname(src.script);
               dst->setCurrentFunction(src.function);
            }
         }
      }
   #pragma endregion
   this->ui.flagInterruptible->setChecked(src_form.scene_flags & loaded_form_type::scene_flag::interruptible);
   this->ui.flagLooping->setChecked(src_form.scene_flags & loaded_form_type::scene_flag::loop_while_conditions_are_met);

   this->ui.conditions->importFrom(src_form, src_form.loop_conditions);
}
void FormSubdialogSceneProperties::exportData(loaded_form_type& dst_form) const {
   #pragma region Papyrus
      this->ui.scriptListPane->commit();
      {  // Fragments
         using fragment_optional_type = decltype(decltype(papyrus_fragment_data_type::fragments)::on_begin);

         bool any_data = false;
         {
            auto* widget_a = this->ui.fragmentBegin;
            auto* widget_b = this->ui.fragmentEnd;
            if (!widget_a->currentScriptname().isEmpty())
               any_data = true;
            else if (!widget_b->currentScriptname().isEmpty())
               any_data = true;
         }

         auto* fragdata = (papyrus_fragment_data_type*) dst_form.script_data.fragment_data;
         if (fragdata) {
            assert(fragdata->type == dovah::loaded_forms::components::papyrus::fragment_type::info);
         } else if (any_data) {
            dst_form.script_data.fragment_data = new papyrus_fragment_data_type;
         }
         if (fragdata) {
            auto _write = [](const DKPapyrusFragmentFunctionPicker* src, fragment_optional_type& dst) {
               auto scriptname = src->currentScriptname();
               auto function   = src->currentFunction();
               if (function.isEmpty()) {
                  dst = {};
               } else {
                  auto& dst_data = dst.emplace();
                  dst_data.script   = scriptname.toStdString();
                  dst_data.function = function.toStdString();
               }
            };
            _write(this->ui.fragmentBegin, fragdata->fragments.on_begin);
            _write(this->ui.fragmentEnd,   fragdata->fragments.on_end);
         }
      }
   #pragma endregion

   cobb::edit_bit(dst_form.scene_flags, loaded_form_type::scene_flag::interruptible, this->ui.flagInterruptible->isChecked());
   cobb::edit_bit(dst_form.scene_flags, loaded_form_type::scene_flag::loop_while_conditions_are_met, this->ui.flagLooping->isChecked());

   this->ui.conditions->exportTo(dst_form, dst_form.loop_conditions);
}