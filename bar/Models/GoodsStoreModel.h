#pragma once
#include <QtSql\qsqltablemodel.h>

class QGoodsStoreModels: public QSqlTableModel
  {
  public:
    explicit QGoodsStoreModels(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
    ~QGoodsStoreModels();

    int GoodNumber(int good_id) const;
    double GoodPrice(int good_id) const;
  };