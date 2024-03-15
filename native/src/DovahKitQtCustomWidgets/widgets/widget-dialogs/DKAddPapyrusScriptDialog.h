#pragma once
#include <QDialog>
#include "dovah/form_types.h"

class DKAddPapyrusScriptModel;
class QCheckBox;
class QListView;
class QLineEdit;

class DKAddPapyrusScriptDialog : public QDialog {
   Q_OBJECT;
   public:
      DKAddPapyrusScriptDialog(QWidget* parent = nullptr);
      
      dovah::form_type targetType() const noexcept;
      void setTargetType(dovah::form_type);

      void setAlreadyAttachedScripts(const std::vector<std::string>&);

      constexpr const std::string& resultScriptname() const noexcept {
         return this->_result;
      }

   protected:
      DKAddPapyrusScriptModel* _model = nullptr;
      struct {
         QListView* listview = nullptr;
         QLineEdit* search   = nullptr;
         QCheckBox* hidden   = nullptr;
      } _subwidgets;
      std::string _result;
};
