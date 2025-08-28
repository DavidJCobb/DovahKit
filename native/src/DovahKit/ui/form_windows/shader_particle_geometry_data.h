#pragma once
#include "./_base.h"
#include "dovah/forms/ShaderParticleGeometry.h"
#include "ui_shader_particle_geometry_data.h"

class FormDialogShaderParticleGeometryData :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::ShaderParticleGeometry, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogShaderParticleGeometryData(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogShaderParticleGeometryData ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
