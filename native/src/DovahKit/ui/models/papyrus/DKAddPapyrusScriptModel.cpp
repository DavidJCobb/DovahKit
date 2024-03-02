#include "./DKAddPapyrusScriptModel.h"
#include <cassert>
#include "./DKAllPapyrusScriptsModel.h"

DKAddPapyrusScriptModel::DKAddPapyrusScriptModel(QObject* parent) : QSortFilterProxyModel(parent) {
   auto& src_model = DKAllPapyrusScriptsModel::get_or_create();
   this->setSourceModel(&src_model);

   this->setSortCaseSensitivity(Qt::CaseInsensitive);
   this->sort(0);
}

void DKAddPapyrusScriptModel::setTargetType(dovah::form_type_t type) {
   if (this->targetType() == type)
      return;
   this->_target_type = type;
   this->invalidateFilter();
}

void DKAddPapyrusScriptModel::setSearchText(QString text) {
   if (this->_search == text)
      return;
   this->_search = text;
   this->invalidateFilter();
}

void DKAddPapyrusScriptModel::setShowHiddenScripts(bool v) {
   if (v == this->showHiddenScripts())
      return;
   this->_show_hidden = v;
   this->invalidateFilter();
}

void DKAddPapyrusScriptModel::setAlreadyAttachedScripts(const std::vector<std::string>& scriptnames) {
   std::vector<QString> transfer;
   size_t size = scriptnames.size();
   transfer.resize(size);
   for (size_t i = 0; i < size; ++i)
      transfer[i] = QString::fromStdString(scriptnames[i]);

   this->_already_attached = transfer;
   this->invalidateFilter();
}

/*virtual*/ bool DKAddPapyrusScriptModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const /*override*/ {
   DKAllPapyrusScriptsModel* src_model;
   #if _DEBUG
      src_model = dynamic_cast<DKAllPapyrusScriptsModel*>(this->sourceModel());
      assert(src_model != nullptr);
   #else
      src_model = (DKAllPapyrusScriptsModel*)this->sourceModel();
   #endif

   auto qmi  = src_model->index(source_row, 0, source_parent);
   auto name = src_model->data(qmi, Qt::DisplayRole).toString();
   if (!this->_search.isEmpty() && !name.contains(this->_search, Qt::CaseInsensitive))
      return false;
   for (auto& unwanted : this->_already_attached)
      if (name.compare(unwanted, Qt::CaseInsensitive) == 0)
         return false;

   if (!this->_show_hidden && src_model->data(qmi, DKAllPapyrusScriptsModel::IsHiddenRole).toBool())
      return false;

   return src_model->scriptIsAttachableTo(qmi, this->_target_type);
}

/*virtual*/ QVariant DKAddPapyrusScriptModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   if (role != Qt::DisplayRole)
      return {};
   if (section != 0)
      return {};
   return tr("Script name");
}
