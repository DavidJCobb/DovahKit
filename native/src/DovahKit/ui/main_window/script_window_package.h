#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_script_window_package.h"
#include "../../editor/script_packages/manifest.h"

class EditorScriptPackageWindow : public QDialog {
   Q_OBJECT
   //
   public:
      EditorScriptPackageWindow(QWidget* parent = Q_NULLPTR);
      
   private:
      Ui::EditorScriptPackageWindow ui;
      struct {
         bool eval_pending   = false;
         bool script_running = false;
      } state;

      QVector<script_packages::manifest> script_packages;

   public slots:
      void reloadPackageList();
      void redrawPackage();
      //
      void runCurrentPackage();
      void logMessage(const QString&);

   protected:
      script_packages::manifest* _getSelectedManifest() noexcept;

      void _onScriptStartStop(bool script_running);
      void _updateEvalEnableState();

      bool _checkAllowClose();
      virtual void closeEvent(QCloseEvent* event) override;
      virtual void reject() override; // override needed to handle Esc key
};
