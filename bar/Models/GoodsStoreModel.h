#pragma once
#include <QtSql\qsqltablemodel.h>

class QGoodsStoreModels: public QSqlTableModel
  {
  public:
    explicit QGoodsStoreModels(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
    ~QGoodsStoreModels();

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    int GoodNumber(int good_id) const;
    double GoodPrice(int good_id) const;
    QString GoodName(int good_id) const;
  };