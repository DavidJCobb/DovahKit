#include "./DKScriptObjectDialog.h"
#include <array>
#include <QComboBox>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPushButton>
#include "helpers/keyboard/key.h"

DKScriptObjectDialog::DKScriptObjectDialog(QWidget& parent, QModelIndex scriptModelIndex) : QDialog(&parent) {
   this->ui.setupUi(this);

   this->setWindowFlag(Qt::WindowContextHelpButtonHint);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);

   this->scriptQMI = scriptModelIndex;
   {
      auto* prop_view = this->ui.properties;
      if (scriptModelIndex.isValid()) {
         prop_view->setModel((QAbstractItemModel*)scriptModelIndex.model());
         prop_view->setRootIndex(scriptModelIndex);
      }
      if (auto* header = prop_view->horizontalHeader()) {
         header->setStretchLastSection(true);
      }
      if (auto* vh = prop_view->verticalHeader()) {
         vh->setSectionResizeMode(QHeaderView::ResizeToContents); // needed for sane row sizing
         vh->setVisible(false);
      }
   }

   // TODO: Simply reusing the same VMAD model doesn't work for changing properties within the dialog 
   //       with OK/Cancel support. We'd have to be able to store "pending" state per-Property, and 
   //       then explicitly commit it...
   //
   //       Almost wonder if a QIdentityProxyModel might be best for that?
   //
   //       Hm... Problem is, we'd have to reimplement the logic for stringifying values, no? Unless 
   //       we specifically expose that somewhere, somehow...
}