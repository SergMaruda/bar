#pragma once
#include <QtSql\qsqltablemodel.h>


class GoodsCheckModel: public QSqlTableModel
  {
    Q_OBJECT
  public:

    explicit GoodsCheckModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const;
private Q_SLOTS:
    void _dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
  };