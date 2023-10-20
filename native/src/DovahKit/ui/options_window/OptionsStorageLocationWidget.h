#pragma once
#include <QDir>
#include <QString>
#include <QWidget>

class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

class OptionsStorageLocationWidget : public QWidget {
   Q_OBJECT;
   public:
      OptionsStorageLocationWidget(QWidget* parent = nullptr);

      void setName(QString);
      void setDescription(QString);
      void setFilePath(QDir);
      void setFolderPath(QDir);

      QString name() const;
      QString description() const;
      bool isFile() const;
      bool isFolder() const;
      QDir path() const;

   protected:
      QString _name;
      QString _description;
      QDir _path;
      bool _isFile = true;
      struct {
         QGroupBox*   groupbox    = nullptr;
         QLineEdit*   path        = nullptr;
         QLabel*      description = nullptr;
         QPushButton* open_folder = nullptr;
         QPushButton* open_file   = nullptr;
      } _subwidgets;

      void _setIsFile(bool);
};