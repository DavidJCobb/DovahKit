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

namespace {
   constexpr const bool allow_incomplete_polishing = true;
}

DKScriptObjectDialog::DKScriptObjectDialog(QWidget& parent, QModelIndex scriptModelIndex) : QDialog(&parent) {
   this->ui.setupUi(this);

   this->setWindowFlag(Qt::WindowContextHelpButtonHint);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);

   static_assert(allow_incomplete_polishing, "POLISH: Show scriptname and form it's attached to in the window title.");

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
}