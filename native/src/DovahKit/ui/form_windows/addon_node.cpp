#include "./addon_node.h"
#include <limits>
#include "dovah/core.h"
#include "editor/helpers/nif_is_valid_master_particle_system.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

#include "dovah/files/bsa/bsa_archived_file.h"
#include "editor/subsystems/assets.h"
#include "nif/file.h"
namespace {
   bool _validate_nif(const std::string& model_path) {
      std::filesystem::path path = std::string("meshes") + (model_path[0] == '/' || model_path[0] == '\\' ? "" : "\\") + model_path;

      std::unique_ptr<dovah::bsa_archived_file> file(dovahkit::subsystems::assets::get().lookup_game_asset(path));
      if (!file)
         return false;

      auto nif = std::make_unique<nifDK::file>();
      nif->read((void*)file->data(), file->size());
      auto& error = nif->read_error();
      if (error.code != nifDK::default_notice_code) {
         #if _DEBUG
            __debugbreak();
         #endif
         return false;
      }
      return editor_helpers::nif_is_valid_master_particle_system(*nif);
   }
}

FormDialogAddOnNode::FormDialogAddOnNode(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.sound->setAllowedFormType(dovah::form_type::sound_descriptor);

   ui::set_range<decltype(loaded_form_type::unique_id)>(this->ui.addonID);
   ui::set_range<decltype(loaded_form_type::master_particle_system_cap)>(this->ui.masterParticleSystemCap);

   QObject::connect(this->ui.model, &DKFormNIFPicker::dataChanged, this, &FormDialogAddOnNode::_update_mps_flag);

   this->load(); // this creates the working copy.
}

void FormDialogAddOnNode::_update_mps_flag() {
   if (!this->form)
      return;

   auto& working  = *this->form;
   auto& dst_flag = working.addon_flags.is_valid_master_particle_system;
   auto& path     = this->ui.model->value().model_path;
   if (path.empty()) {
      dst_flag = false;
   } else {
      dst_flag = _validate_nif(path);
   }
   this->ui.masterParticleSystemCap->setEnabled(dst_flag);
}

void FormDialogAddOnNode::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.addonID, working.unique_id);
   this->ui.model->initializeFrom(working.model);
   ui::bind(this->ui.sound, working.sound, working);
   ui::bind(this->ui.masterParticleSystemCap, working.master_particle_system_cap);
   ui::bind(this->ui.flagAlwaysLoaded, working.addon_flags.always_loaded);

   this->_update_mps_flag();
}
void FormDialogAddOnNode::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   this->ui.model->commitTo(working.model, working);
}