#include "./FormSubdialogLensFlareSprite.h"
#include "ui/utils/bind.h"

FormSubdialogLensFlareSprite::FormSubdialogLensFlareSprite(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   ui::bind(this->ui.texture, this->ui.texturePreview);
}

FormSubdialogLensFlareSprite::value_type FormSubdialogLensFlareSprite::value() const {
   value_type dst;
   dst.id = this->ui.editorID->text().toStdString();
   dst.data.width = this->ui.width->value();
   dst.data.height = this->ui.height->value();
   dst.data.position = this->ui.position->value();
   dst.data.angular_fade = this->ui.angularFade->value();
   dst.data.opacity = this->ui.opacity->value();
   {
      auto&  dst_c = dst.data.tint;
      QColor src_c = this->ui.color->color();
      dst_c.r = src_c.redF();
      dst_c.g = src_c.greenF();
      dst_c.b = src_c.blueF();
   }
   dst.texture = this->ui.texture->value().to_string().toStdString();
   
   dst.data.flags = 0;
   if (this->ui.flagRotates->isChecked())
      dst.data.flags |= value_type::flag::rotates;
   if (this->ui.flagShrinkWhenOccluded->isChecked())
      dst.data.flags |= value_type::flag::shrinks_when_occluded;

   return dst;
}
void FormSubdialogLensFlareSprite::setValue(const value_type& src) {
   this->ui.editorID->setText(QString::fromStdString(src.id));
   this->ui.width->setValue(src.data.width);
   this->ui.height->setValue(src.data.height);
   this->ui.position->setValue(src.data.position);
   this->ui.angularFade->setValue(src.data.angular_fade);
   this->ui.opacity->setValue(src.data.opacity);
   {
      QColor c;
      c.setRedF(src.data.tint.r);
      c.setGreenF(src.data.tint.g);
      c.setBlueF(src.data.tint.b);
      this->ui.color->setColor(c);
   }
   this->ui.texture->setValue(ui::types::game_file_path(src.texture.c_str()));

   this->ui.flagRotates->setChecked(src.data.flags & value_type::flag::rotates);
   this->ui.flagShrinkWhenOccluded->setChecked(src.data.flags & value_type::flag::shrinks_when_occluded);
}