#pragma once
#include <QSortFilterProxyModel>
#include "dovah/form_types.h"

// Model for the list of available scripts shown when attempting to add a new script to a 
// form or alias.
//
class DKAddPapyrusScriptModel : public QSortFilterProxyModel {
   Q_OBJECT;
   public:
      DKAddPapyrusScriptModel(QObject* parent = nullptr);

   protected:
      using QSortFilterProxyModel::setSourceModel;

      std::vector<QString> _already_attached;
      dovah::form_type     _target_type = dovah::form_type::none;

      bool    _show_hidden = false;
      QString _search;

   public:
      constexpr dovah::form_type targetType() const noexcept { return this->_target_type; }
      void setTargetType(dovah::form_type);

      QString searchText() const noexcept { return this->_search; }
      void setSearchText(QString);

      constexpr bool showHiddenScripts() const noexcept { return this->_show_hidden; }
      void setShowHiddenScripts(bool);

      void setAlreadyAttachedScripts(const std::vector<std::string>&);

      virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

      virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
};