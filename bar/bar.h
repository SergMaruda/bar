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
    QSqlTableModel* goods_check_view;
    QSqlDatabase m_db;
    void _ReloadModel(QSqlTableModel*);
    double _getNewestPurchasePrice(int id);
    QString _GetUserPassword(int id);
private Q_SLOTS:

  void OnDoubleClick(QModelIndex);
  void OnOnConfirmSale();
  void OnOnConfirmPurchase();
  void OnOnChangeCash();
  void UpdateChange();
  void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
  void NoGoodToolTip();
  void _UpdatePurchasePrice();
  void _UpdateCash();
  void _UpdateOrderPrice();
  QListWidgetItem* _findGoodItem(int good_id);
  void _SyncGoodsIcons();
  void _UpdateTakeOffMode();
  void _PerformGoodsCheck();
};

#endif // BAR_H
