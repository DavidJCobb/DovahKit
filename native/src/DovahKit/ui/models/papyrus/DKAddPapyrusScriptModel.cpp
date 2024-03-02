#include "./DKAddPapyrusScriptModel.h"
#include <cassert>
#include "./DKAllPapyrusScriptsModel.h"

DKAddPapyrusScriptModel::DKAddPapyrusScriptModel(QObject* parent) : QSortFilterProxyModel(parent) {
   auto& src_model = DKAllPapyrusScriptsModel::get_or_create();
   this->setSourceModel(&src_model);
}

void DKAddPapyrusScriptModel::setTargetType(dovah::form_type_t type) {
   if (this->targetType() == type)
      return;
   this->_target_type = type;
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
   for (auto& unwanted : this->_already_attached)
      if (name.compare(unwanted, Qt::CaseInsensitive) == 0)
         return false;

   return src_model->scriptIsAttachableTo(qmi, this->_target_type);
}
