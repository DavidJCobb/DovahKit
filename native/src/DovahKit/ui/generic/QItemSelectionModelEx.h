#pragma once
#include <QItemSelectionModel>

//
// Subclass of QItemSelectionModel which allows you to distinguish between 
// user-initiated selections and automated deselections (the latter including 
// things like "deselecting a row that is about to be removed from the table").
//
class QItemSelectionModelEx : public QItemSelectionModel {
   Q_OBJECT;
   protected:
      void _init();
      bool _user_initiated = false;

   public:
      QItemSelectionModelEx(QAbstractItemModel*, QObject*);
      QItemSelectionModelEx(QAbstractItemModel* model = nullptr);

      virtual void select(const QItemSelection&, SelectionFlags) override;

   signals:
      void userInitiatedSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

   protected slots:
      void _onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
};