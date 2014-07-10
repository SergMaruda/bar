#ifndef BAR_H
#define BAR_H

#include <QtWidgets/QMainWindow>
#include "ui_bar.h"
#include <QtSql/qsqldatabase.h>

class QSqlTableModel;
class QSqlDatabase;
class QSqlQueryModel;
class QGoodsStoreModels;

class bar : public QMainWindow
{
    Q_OBJECT

public:
    bar(QWidget *parent = 0);
    ~bar();

private:
    Ui::barClass ui;
    QGoodsStoreModels* model_goods;
    QSqlTableModel* model_transactions_view;
    QSqlTableModel* model_transactions;
    QSqlQueryModel* model_order_price;
    QSqlQueryModel* model_purchase_price;
    QSqlQueryModel* model_cash;
    QSqlTableModel* model_users;
    QSqlDatabase m_db;
    void _ReloadModel(QSqlTableModel*);
private Q_SLOTS:
  void OnDoubleClick(QModelIndex);
  void OnOnConfirmSelling();
  void OnOnConfirmPurchase();
  void OnOnChangeCash();
  void UpdateChange();
  void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
  void NoGoodToolTip();
  void _UpdatePurchasePrice();
  void _UpdateCash();
  void _UpdateOrderPrice();
};

#endif // BAR_H
