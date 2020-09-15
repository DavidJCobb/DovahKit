#include "load_window.h"

//
// TODO:
//  - Sort file list by load order (file_list.cpp)
//  - Double-clicking an entry in the file list should select it
//  - Support for (de)selecting an active file
//     - Button should serve as a toggle
//     - Change button label depending on whether the selected file is(n't) the active file (CK doesn't do this but we should)
//  - Make stuff actually happen when you click the "load" button
//

LoadOrderOpenDialog::LoadOrderOpenDialog(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   this->ui.dateCreated->setText("");
   this->ui.dateModified->setText("");
   QObject::connect(this->ui.fileList->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
      this->ui.author->setText("");
      this->ui.description->setPlainText("");
      this->ui.dateCreated->setText("");
      this->ui.dateModified->setText("");
      this->ui.dependencies->clear();
      //
      auto widget = this->ui.fileList;
      auto select = widget->selectionModel()->selection();
      for (const auto& idx : select.indexes()) {
         const auto* item = (LoadOrderFileList::model_item_type*)idx.internalPointer();
         if (!item)
            continue;
         this->ui.author->setText(item->author);
         this->ui.description->setPlainText(item->description);
         this->ui.dateCreated->setText(item->created.toString());
         this->ui.dateModified->setText(item->modified.toString());
         //
         auto& list = item->dependencies;
         auto  size = item->dependencies.size();
         for (size_t i = 0; i < size; ++i) {
            this->ui.dependencies->insertItem(i, list[i]);
         }
         break;
      }
   });
   //
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, [this]() {
      this->reject();
   });
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, [this]() {
      this->commit();
      this->accept();
   });
}
void LoadOrderOpenDialog::commit() {
   //
   // TODO
   //
}