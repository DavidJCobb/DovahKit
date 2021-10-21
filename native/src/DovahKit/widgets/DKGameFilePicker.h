#pragma once
#include <QLineEdit>
#include <QPushButton>

class DKGameFilePicker : public QWidget {
   Q_OBJECT;
   public:
      enum PathFormat {
         WidgetPathFormat,  // "textures/foo/bar.dds"
         ShowDataDirectory, // "data/textures/foo/bar.dds"
         OmitPathStem,      // "foo/bar.dds", given a path stem "textures" or "textures/"
      };
      Q_ENUM(PathFormat);

      enum StandardConfiguration { // a quick way to assign common defaults to any properties left blank e.g. the path stem
         NoConfiguration,
         Meshes,
         Textures,
      };
      Q_ENUM(StandardConfiguration);

      enum ValidationOption {
         ArmorAddonModel = 0x01,
      };
      Q_DECLARE_FLAGS(ValidationOptions, ValidationOption);
      Q_FLAG(ValidationOptions);

      Q_PROPERTY(QString    path       READ path       WRITE setPath       DESIGNABLE true USER true); // Current path, using the WidgetPathFormat path format (i.e. the widget's internal representation of the path).
      Q_PROPERTY(PathFormat pathFormat READ pathFormat WRITE setPathFormat DESIGNABLE true);           // Control how the path is displayed in the control, and how it's exposed when rawPath is called.
      Q_PROPERTY(QString    pathStem   READ pathStem   WRITE setPathStem   DESIGNABLE true);           // Limit file selection, disallowing files not in this folder.
      Q_PROPERTY(StandardConfiguration standardConfiguration READ standardConfiguration WRITE setStandardConfiguration DESIGNABLE true);
      Q_PROPERTY(ValidationOptions     validationOptions     READ validationOptions     WRITE setValidationOptions     DESIGNABLE true);

   public:
      DKGameFilePicker(QWidget* parent);

      inline bool isEmpty() const noexcept { return this->state.value.isEmpty(); }

      inline QString path() const noexcept { return this->state.value; }
      inline PathFormat pathFormat() const noexcept { return this->state.displayFormat; }
      inline QString pathStem() const noexcept { return this->state.stem; }
      inline StandardConfiguration standardConfiguration() const noexcept { return this->state.config; }
      inline ValidationOptions validationOptions() const noexcept { return this->state.validationOptions; }

      QString effectiveStem() const noexcept; // pathStem, or the stem provided as part of a StandardConfiguration
      QString path(PathFormat v) const noexcept;
      QString rawPath() const noexcept; // applies the current PathFormat, and uses backslashes for directory separators

      static QString normalizePath(const QString&) noexcept; // attempts to convert a path to WidgetPathFormat

   public slots:
      void browse(); // open the browse dialog
      void clear();
      void setPath(const QString&);
      void setPathFormat(PathFormat);
      void setPathStem(const QString&); // does not retroactively update the current path
      void setRawPath(const QString&);
      void setStandardConfiguration(StandardConfiguration); // overwrites the path stem, etc., as appropriate
      void setValidationOptions(ValidationOptions);

   signals:
      void pathChanged(const QString&);
      void rawPathChanged(const QString&);

   protected:
      struct {
         QLineEdit*   path   = nullptr;
         QPushButton* browse = nullptr;
      } subwidgets;
      struct {
         StandardConfiguration config = StandardConfiguration::NoConfiguration;
         PathFormat displayFormat = PathFormat::WidgetPathFormat;
         QString stem; // e.g. to require a texture, you'd set this to "textures/"
         QString value;
         ValidationOptions validationOptions;
      } state;

      virtual bool eventFilter(QObject* watched, QEvent* event) override;

      void _emitPathChangeSignals();
      void _setPath(const QString& path, bool isRawPath, bool updateTextbox);
      void _updateBrowseButtonSize();
};