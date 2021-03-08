#pragma once
#include <QStandardItemModel>

class QStandardItemModelDKEx : public QStandardItemModel {
   protected:
      bool _auto_tooltip = false;
      //
      void _forceUpdateAll(const QVector<int>& roles = QVector<int>());
      //
   public:
      using QStandardItemModel::QStandardItemModel;

      inline bool autoTooltips() const noexcept { return this->_auto_tooltip; }
      void setAutoTooltips(bool);

      // QStandardItemModel::clear erases the headers, not just the normal data. This erases only 
      // the normal data.
      inline void clearBody() {
         this->removeRows(0, this->rowCount());
      }

      virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};