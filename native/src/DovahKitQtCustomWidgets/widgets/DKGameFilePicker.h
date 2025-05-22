#pragma once
#include <QLineEdit>
#include <QPushButton>
#include "ui/types/game_file_path.h"

class DKGameFilePicker : public QWidget {
   Q_OBJECT;
   public:
      // Preset configurations for the path stem, allowed file extensions, etc..
      enum PresetConfiguration {
         NoConfiguration,
         Meshes,
         Sounds,
         Textures,
      };
      Q_ENUM(PresetConfiguration);
      
      enum SpecialValidation {
         NoSpecialValidation,
         ArmorAddonModel,
      };
      Q_ENUM(SpecialValidation);

      Q_PROPERTY(QStringList allowedFileExtensions READ allowedFileExtensions WRITE setAllowedFileExtensions DESIGNABLE true);

      Q_PROPERTY(bool displaysDataDirectory READ displaysDataDirectory WRITE setDisplaysDataDirectory DESIGNABLE true);
      Q_PROPERTY(bool displaysStem READ displaysStem WRITE setDisplaysStem DESIGNABLE true);
      Q_PROPERTY(bool displaysWithBackslashes READ displaysWithBackslashes WRITE setDisplaysWithBackslashes DESIGNABLE true);
      
      Q_PROPERTY(QString pathStem READ pathStemString WRITE setPathStemString DESIGNABLE true);
      
      Q_PROPERTY(PresetConfiguration presetConfiguration READ presetConfiguration WRITE setPresetConfiguration DESIGNABLE true);
      Q_PROPERTY(SpecialValidation specialValidation READ specialValidation WRITE setSpecialValidation DESIGNABLE true);
      
   public:
      DKGameFilePicker(QWidget* parent = nullptr);

      #if !defined(QT_PLUGIN)
         // If you set a relative path, it'll be treated as relative to the Data 
         // directory and normalized accordingly.
         ui::types::game_file_path value();
         void setValue(ui::types::game_file_path);
      #endif
      
      // The path stem is the required start of the path, including the root 
      // Data directory (if you set it to a value that lacks said directory, 
      // then we add it). The user will only be allowed to pick files that are 
      // somewhere within the stem.
      //
      // If you pass in a relative path, it'll be treated as relative to the 
      // Data directory.
      ui::types::game_file_path pathStem() const;
      void setPathStem(ui::types::game_file_path);
      
      // For Qt Designer:
      QString pathStemString() const;
      void setPathStemString(QString);
      
      #pragma region Configure the display path format
         bool displaysDataDirectory() const;
         void setDisplaysDataDirectory(bool);
         
         bool displaysStem() const;
         void setDisplaysStem(bool);
         
         bool displaysWithBackslashes() const;
         void setDisplaysWithBackslashes(bool);
      #pragma endregion
      
      SpecialValidation specialValidation() const;
      void setSpecialValidation(SpecialValidation);
      
      // NOTE: Setting `NoConfiguration` won't revert any already-set options.
      PresetConfiguration presetConfiguration() const;
      void setPresetConfiguration(PresetConfiguration);
      
      // An empty list means no limit is being applied. Leading periods will 
      // be stripped and values will be forced to lowercase (i.e. ".WAV" will 
      // be stored as "wav").
      QStringList allowedFileExtensions() const;
      void setAllowedFileExtensions(QStringList);
      void allowFileExtension(QStringView);
      
   public slots:
      void browse(); // open the browse dialog
      void clear();
      
   signals:
      void valueChanged(ui::types::game_file_path);
      
   protected:
      struct {
         PresetConfiguration preset_config      = PresetConfiguration::NoConfiguration;
         SpecialValidation   special_validation = SpecialValidation::NoSpecialValidation;
         struct : ui::types::game_file_path::format_options {
            bool include_stem = true;
         } display_path;
         ui::types::game_file_path value;
         ui::types::game_file_path stem;
         QStringList allowed_file_extensions;
      } _properties;
      struct {
         QLineEdit*   path   = nullptr;
         QPushButton* browse = nullptr;
      } _subwidgets;

      virtual bool eventFilter(QObject* watched, QEvent* event) override;
      
      void _onAllowedFileExtensionsChanged();
      void _onDisplayPathFormatChanged();
      void _revalidatePath(bool silent);
      void _updateBrowseButtonSize();
      void _updateDisplayedPath();
      
};