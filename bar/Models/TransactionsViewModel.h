#pragma once
#include <QtSql\qsqltablemodel.h>

namespace TransactionsViewColumn
  {
  enum EColumns
    {
    Number = 2,
    Price = 3,
    Sum = 4
    };
  };

class TransactionsViewModel: public QSqlTableModel
  {
  public:
    explicit TransactionsViewModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase()):
    QSqlTableModel(parent, db)
      {
      }

    virtual bool selectRow(int row);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const;
  };