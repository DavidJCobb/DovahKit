#pragma once
#include <functional>
#include <QSortFilterProxyModel>

//
// Helper class. Allows you to use lambdas to override behaviors, instead of having to make 
// subclasses for every case.
//
class QSortFilterProxyModelDKEx : public QSortFilterProxyModel {
   Q_OBJECT;
   protected:
      using less_than_handler_t     = std::function<bool(const QSortFilterProxyModelDKEx&, const QModelIndex&, const QModelIndex&)>;
      using filter_accept_handler_t = std::function<bool(const QSortFilterProxyModelDKEx&, int, const QModelIndex&)>;
      //
   public:
      QSortFilterProxyModelDKEx(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}
      
      void setLessThanFunction(less_than_handler_t);
      void setFilterAcceptsColumnFunction(filter_accept_handler_t);
      void setFilterAcceptsRowFunction(filter_accept_handler_t);
      
      // Access to the original superclass methods, so that functors can fall back to the 
      // default behavior conditionally.
      bool defaultLessThanFunction(const QModelIndex&, const QModelIndex&) const;
      bool defaultFilterAcceptsColumn(int col, const QModelIndex& parent) const;
      bool defaultFilterAcceptsRow(int row, const QModelIndex& parent) const;
      
   protected:
      struct {
         less_than_handler_t     lessThan;
         filter_accept_handler_t filterAcceptsColumn;
         filter_accept_handler_t filterAcceptsRow;
      } _functors;
      //
      virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
      virtual bool filterAcceptsColumn(int col, const QModelIndex& parent) const override;
      virtual bool filterAcceptsRow(int row, const QModelIndex& parent) const override;
};
