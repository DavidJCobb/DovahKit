#include "./DKGameFilePicker.h"
#include <cassert>
#include <QAction>
#include <QDir>
#include <QEvent>
#include <QHBoxLayout>
#include <QStyleOptionButton>
#if !defined(QT_PLUGIN)
   #include "./widget-dialogs/DKBSABrowseDialog.h"
#endif

static bool _string_starts_with_data_dir(QStringView v);

DKGameFilePicker::DKGameFilePicker(QWidget* parent) : QWidget(parent) {
   auto* layout   = new QHBoxLayout(this);
   auto* w_path   = this->_subwidgets.path   = new QLineEdit(this);
   auto* w_browse = this->_subwidgets.browse = new QPushButton(tr("...", "browse button label"), this);
   layout->setContentsMargins({ 0, 0, 0, 0 });
   layout->addWidget(w_path, 1);
   layout->addWidget(w_browse, 0);
   #if !defined(QT_PLUGIN)
      QObject::connect(w_browse, &QPushButton::clicked,  this, &DKGameFilePicker::browse);
      QObject::connect(w_path,   &QLineEdit::textEdited, this, [this](const QString& path) {
         ui::types::game_file_path value(path);
         {
            auto& stem = this->_properties.stem;
            if (!this->_properties.display_path.include_stem) {
               if (value.is_absolute()) {
                  value = stem.append(value.lexically_relative(ui::types::game_file_path("Data\\")));
               }
            }
         }
         this->setValue(value);
      });
   #endif
   this->_updateBrowseButtonSize();
   //
   w_browse->setAccessibleName(tr("Browse..."));
   w_browse->setAccessibleDescription(tr("Open a dialog to select a game asset file from the currently loaded BSA archives."));
   //
   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(w_path);
   QWidget::setTabOrder(w_path, w_browse);
}

#if !defined(QT_PLUGIN)
   ui::types::game_file_path DKGameFilePicker::value() {
      return this->_properties.value;
   }
   void DKGameFilePicker::setValue(ui::types::game_file_path v) {
      if (!v.is_absolute()) {
         v = ui::types::game_file_path("Data\\").append(v);
      }
      auto prior = this->value();
      this->_properties.value = v;
      this->_revalidatePath(false);
      this->_updateDisplayedPath(); // always re-display, in case letter case changed
      if (prior != this->_properties.value) {
         emit this->valueChanged(v);
      }
   }
#endif

ui::types::game_file_path DKGameFilePicker::pathStem() const {
   return this->_properties.stem;
}
void DKGameFilePicker::setPathStem(ui::types::game_file_path v) {
   if (v.is_relative()) {
      v = ui::types::game_file_path("Data\\").append(v);
   }
   if (v.has_filename()) {
      v.append(ui::types::game_file_path{});
   }
   auto prior = this->_properties.stem;
   this->_properties.stem = v;

   #if !defined(QT_PLUGIN)
      if (!v.empty() && this->_properties.value.lexically_relative(v).empty()) {
         //
         // Our current value doesn't start with the new stem; clear it.
         //
         this->clear();
      } else {
         this->_onDisplayPathFormatChanged(); // in case the letter casing changed
      }
   #endif
}

QString DKGameFilePicker::pathStemString() const {
   return this->_properties.stem.to_string();
}
void DKGameFilePicker::setPathStemString(QString v) {
   this->setPathStem(ui::types::game_file_path(v));
}

#pragma region Slots
   void DKGameFilePicker::browse() {
      #if !defined(QT_PLUGIN)
         QString filter;
         if (auto& list = this->_properties.allowed_file_extensions; !list.isEmpty()) {
            filter = "Game Files (";
            for(size_t i = 0; i < list.size(); ++i) {
               if (i)
                  filter += ' ';
               filter += '*';
               filter += list[i];
            }
            filter += ')';
         }
         
         //
         // DKBSABrowseDialog uses paths without the Data directory, with 
         // forward slashes. (TODO: Update it to use ui::types::game_file_path.)
         //
         QString browse_stem;
         QString browse_initial;
         browse_stem = this->_properties.stem.to_string({
            .include_data_directory = false,
            .use_backslashes        = false,
         });
         browse_initial = this->_properties.value.to_string({
            .include_data_directory = false,
            .use_backslashes        = false,
         });
      
         auto result = DKBSABrowseDialog::getOpenFileName(
            this,
            tr("Select file"),
            browse_stem,
            browse_initial,
            filter, // TODO: Not yet implemented in DKBSABrowseDialog and the underlying model.
            nullptr,
            {
               .specialValidation = this->specialValidation(),
            }
         );
         result = QDir::cleanPath(result);
         if (result.isEmpty())
            return;
         
         this->_properties.value = ui::types::game_file_path(
            result,
            {
               .include_data_directory = false,
               .use_backslashes        = false,
            }
         );
      #endif
   }
   void DKGameFilePicker::clear() {
      bool changed = !this->_properties.value.empty();
      this->_properties.value = {};
      this->_updateDisplayedPath();
      if (changed)
         emit this->valueChanged({});
   }
#pragma endregion

#pragma region Properties
   #pragma region Configure the display path format
      bool DKGameFilePicker::displaysDataDirectory() const {
         return this->_properties.display_path.include_data_directory;
      }
      void DKGameFilePicker::setDisplaysDataDirectory(bool v) {
         auto& dst = this->_properties.display_path.include_data_directory;
         if (v == dst)
            return;
         dst = v;
         this->_onDisplayPathFormatChanged();
      }
      
      bool DKGameFilePicker::displaysStem() const {
         return this->_properties.display_path.include_stem;
      }
      void DKGameFilePicker::setDisplaysStem(bool v) {
         auto& dst = this->_properties.display_path.include_stem;
         if (v == dst)
            return;
         dst = v;
         this->_onDisplayPathFormatChanged();
      }
      
      bool DKGameFilePicker::displaysWithBackslashes() const {
         return this->_properties.display_path.use_backslashes;
      }
      void DKGameFilePicker::setDisplaysWithBackslashes(bool v) {
         auto& dst = this->_properties.display_path.use_backslashes;
         if (v == dst)
            return;
         dst = v;
         this->_onDisplayPathFormatChanged();
      }
   #pragma endregion
   
   DKGameFilePicker::SpecialValidation DKGameFilePicker::specialValidation() const {
      return this->_properties.special_validation;
   }
   void DKGameFilePicker::setSpecialValidation(SpecialValidation v) {
      this->_properties.special_validation = v;
   }
   
   DKGameFilePicker::PresetConfiguration DKGameFilePicker::presetConfiguration() const {
      return this->_properties.preset_config;
   }
   void DKGameFilePicker::setPresetConfiguration(PresetConfiguration v) {
      this->_properties.preset_config = v;
      switch (v) {
         case PresetConfiguration::Meshes:
            this->_properties.stem = "Data\\Meshes\\";
            this->_properties.allowed_file_extensions = QStringList{
               ".hkx",
               ".nif",
            };
            break;
         case PresetConfiguration::Sounds:
            this->_properties.stem = "Data\\Sound\\";
            this->_properties.allowed_file_extensions = QStringList{
               ".fuz",
               ".xwm",
               ".wav",
            };
            break;
         case PresetConfiguration::Textures:
            this->_properties.stem = "Data\\Textures\\";
            this->_properties.allowed_file_extensions = QStringList{
               ".dds",
            };
            break;
      }
      this->_revalidatePath(false);
   }
   
   QStringList DKGameFilePicker::allowedFileExtensions() const {
      return this->_properties.allowed_file_extensions;
   }
   void DKGameFilePicker::setAllowedFileExtensions(QStringList list) {
      auto& dst = this->_properties.allowed_file_extensions;
      dst.clear();
      for(QStringView item : list) {
         if (item.isEmpty())
            continue;
         auto ext = item.toString().toLower();
         if (ext[0] != '.')
            ext.insert(0, '.');
         if (!dst.contains(ext))
            dst.push_back(ext);
      }
      this->_onAllowedFileExtensionsChanged();
   }
   void DKGameFilePicker::allowFileExtension(QStringView v) {
      if (v.isEmpty())
         return;
      auto  ext = v.toString().toLower();
      if (ext[0] != '.')
         ext.insert(0, '.');
      auto& dst = this->_properties.allowed_file_extensions;
      if (!dst.contains(ext)) {
         dst.push_back(ext);
         this->_onAllowedFileExtensionsChanged();
      }
   }
#pragma endregion

bool DKGameFilePicker::eventFilter(QObject* watched, QEvent* event) {
   #if !defined(QT_PLUGIN)
      if (watched == this->_subwidgets.path) {
         auto* widget = this->_subwidgets.path;
         if (event->type() == QEvent::Type::FocusOut) {
            this->_updateDisplayedPath();
            return false;
         }
         return false;
      }
   #endif
   if (watched == this->_subwidgets.browse) {
      switch (event->type()) {
         case QEvent::FontChange:
         case QEvent::MacSizeChange:
         case QEvent::StyleChange:
            this->_updateBrowseButtonSize();
            break;
      }
      return false;
   }
   return false;
}

void DKGameFilePicker::_onAllowedFileExtensionsChanged() {
   #if !defined(QT_PLUGIN)
      if (this->_properties.value.empty())
         return;
      if (this->_properties.allowed_file_extensions.isEmpty())
         return;
      auto ext = this->_properties.value.extension();
      for(const auto& allowed : this->_properties.allowed_file_extensions) {
         if (ext == allowed)
            return;
      }
      //
      // The existing path doesn't match any of the allowed extensions. 
      // Clear it out.
      //
      this->clear();
   #endif
}
void DKGameFilePicker::_onDisplayPathFormatChanged() {
   this->_updateDisplayedPath();
}
void DKGameFilePicker::_revalidatePath(bool silent) {
   #if !defined(QT_PLUGIN)
      auto& value = this->_properties.value;
      auto& types = this->_properties.allowed_file_extensions;
      auto& stem  = this->_properties.stem;
      if (value.empty()) {
         return;
      }
      if (!value.has_filename()) {
         value = {};
         if (!silent) {
            this->_updateDisplayedPath();
            emit this->valueChanged({});
         }
         return;
      }
      if (!types.isEmpty()) {
         bool valid = false;
         auto ext   = value.extension();
         for(auto& type : types) {
            if (ext == type) {
               valid = true;
               break;
            }
         }
         if (!valid) {
            value = {};
            if (!silent) {
               this->_updateDisplayedPath();
               emit this->valueChanged({});
            }
            return;
         }
      }
      if (!stem.empty()) {
         if (value.lexically_relative(stem).empty()) {
            value = {};
            if (!silent) {
               this->_updateDisplayedPath();
               emit this->valueChanged({});
            }
            return;
         }
      }
   #endif
}
void DKGameFilePicker::_updateBrowseButtonSize() {
   auto* widget = this->_subwidgets.browse;
   widget->ensurePolished();
   //
   auto* style = widget->style();
   if (!style)
      return;
   auto text  = widget->text();
   auto fm    = QFontMetrics(widget->font());
   auto width = fm.boundingRect(text).width();
   //
   QStyleOptionButton opt;
   opt.initFrom(widget);
   std::swap(opt.text, text);
   auto frame = style->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, widget);
   frame += style->pixelMetric(QStyle::PM_ButtonMargin, &opt, widget);
   frame *= 2;
   //
   widget->setMaximumSize({ frame + width, QWIDGETSIZE_MAX });
}
void DKGameFilePicker::_updateDisplayedPath() {
   #if !defined(QT_PLUGIN)
      QString text;
      {
         auto value = this->_properties.value;
         if (!value.empty()) {
            ui::types::game_file_path::format_options options;
            options.include_data_directory = this->_properties.display_path.include_data_directory;
            options.use_backslashes        = this->_properties.display_path.use_backslashes;
         
            if (!this->_properties.display_path.include_stem && !this->_properties.stem.empty()) {
               value = value.lexically_relative(this->_properties.stem);
            }
            text = value.to_string(options);
         }
      }
   
      auto* widget  = this->_subwidgets.path;
      auto  blocker = QSignalBlocker(widget);
      widget->setText(text);
   #endif
}