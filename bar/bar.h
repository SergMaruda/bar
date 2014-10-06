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
    QSqlQueryModel* depts_model;

    void _ReloadModel(QSqlTableModel*);
    double _getNewestPurchasePrice(int id);
private Q_SLOTS:

  void OnDoubleClick(QModelIndex);
  void MainTabWidgetChanged(int);
  void OnOnConfirmSale();
  void OnOnConfirmPurchase();
  void OnOnChangeCash();
  void UpdateChange();
  void TransactionsDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
  void GoodsCheckDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
  void GoodsStoreDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
  void _UpdatePurchasePrice();
  void _UpdateCash();
  void _UpdateOrderPrice();
  QListWidgetItem* _findGoodItem(int good_id);
  void _SyncGoodsIcons();
  void _UpdateTakeOffMode();
  void _PerformGoodsCheck();
  void _UpdateGoodsCheckMode();
  void _ResetGoodsCheck();
  void _NotifyNoGood();
  void _OnRightClickGoodsView(QPoint pos);
  void _OnAddGood();
  void _OnRemoveGood();
  void _OnRemoveGoodCompletely();
  void _OnAddGoodFilter();
  void _OnResetGoodFilter();
  void _OnDebtsByPersons();
  void _OnDebtsByGood();
  void _OnRemoveDebt();

  void _InitTransactionView(QTableView*);
  void _UpdateStat();
  bool _UpdateQuery(const QString&);
  void _TransactionsViewRightClick(QPoint);
  void _OnRemoveTransaction();
  void _UpdateWhenTransactionsChanged();
  void _IncFont();
  void _DecFont();
  int _fontSize();
  void _setFont(int);
  void _OnExit();
  };

#endif // BAR_H
