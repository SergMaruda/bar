#include "bar.h"
#include "QLoginDialog.h"
#include "QBarApplication.h"
#include "Models\GoodsStoreModel.h"
#include "TransactionStatus.h"
#include "Models\TransactionsViewModel.h"
#include "Models\GoodsCheckModel.h"
#include "UserRoles.h"

#include <QtCore\qdatetime.h>
#include <QtCore\qthread.h>
#include <QtCore\qstandardpaths.h>
#include <QtCore\qprocess.h>
#include <QtCore\qfileinfo.h>
#include <QtCore\qregularexpression.h>
#include <QtCore\qsettings.h>
#include <QtCore\qsortfilterproxymodel.h>
#include <QtCore\QTimer>

#include <QtSql\qsqldatabase.h>
#include <QtSql\qsqltablemodel.h>
#include <QtSql\qsqlrecord.h>
#include <QtSql\qsqlquery.h>
#include <QtSql\qsqlfield.h>
#include <QtSql\qsqlindex.h>
#include <QtSql\qsqlerror.h>
#include <QtSql\QSqlRelationalTableModel.h>

#include <QtWidgets\qscrollbar.h>
#include <QtWidgets\qtooltip.h>
#include <QtWidgets\QMessageBox>
#include <QtWidgets\qheaderview.h>
#include <QtWidgets\QMenu>

#include <QStyledItemDelegate>

//----------------------------------------------------
/*
TODO
-add goods icons in table-
-merge transactions into one table
-split into models



*/


namespace TransactionsView
  {
  enum
    {
    Status = 0,
    TransactionID = 7,
    GoodID = 8
    };
  }

namespace GoodsStoreView
  {
  enum
    {
    ID = 0
    };
  }


//------------------------------------------------------------
bar::bar(QWidget *parent)
    : QMainWindow(parent)
  {
  auto bar_app = QBarApplication::instance();
  model_users = bar_app->model_users();

  ui.setupUi(this);

  int font = bar_app->settings().value("UI/font", 8).toInt();
  _setFont(font);

  ui.tableViewGood->hideColumn(1);

  model_order_price = new QSqlQueryModel(this);
  model_order_price->setQuery("SELECT * FROM OrderPrice");

  model_purchase_price = new QSqlQueryModel(this);
  model_purchase_price->setQuery("SELECT * FROM PurchasePrice");

  model_cash = new QSqlQueryModel(this);
  model_cash->setQuery("SELECT * FROM Cash");

  model_goods = new QGoodsStoreModels(this);

  _ReloadModel(model_goods);

  ui.tableViewGood->setModel(model_goods);
  ui.tableViewGood->hideColumn(0);
  ui.tableViewGood->resizeColumnsToContents();
  ui.tableViewGood->setSortingEnabled(true);
  ui.tableViewGood->scrollToBottom();
 
  QHeaderView *verticalHeader = ui.tableViewGood->verticalHeader();
  verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

  QObject::connect(ui.tableViewGood, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnDoubleClick(QModelIndex)));
  QObject::connect(ui.tabWidgetMain, SIGNAL(currentChanged (int)), this, SLOT(MainTabWidgetChanged(int)));
 
  QObject::connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(OnOnConfirmSale()));
  QObject::connect(ui.pushButtonPurchase, SIGNAL(clicked()), this, SLOT(OnOnConfirmPurchase()));
  QObject::connect(ui.pushButtonPerformCashManipulation, SIGNAL(clicked()), this, SLOT(OnOnChangeCash()));

  QObject::connect(ui.doubleSpinBoxCustomerCash, SIGNAL(valueChanged(double)), this, SLOT(UpdateChange()));
  QObject::connect(ui.lineEditOrderPrice, SIGNAL(textChanged(const QString&)), this, SLOT(UpdateChange()));

  QObject::connect(ui.radioButtonCash, SIGNAL(clicked()), this, SLOT(_UpdateTakeOffMode()));
  QObject::connect(ui.radioButtonGood, SIGNAL(clicked()), this, SLOT(_UpdateTakeOffMode()));
  QObject::connect(ui.performGoodsCheck, SIGNAL(clicked()), this, SLOT(_PerformGoodsCheck()));
  QObject::connect(ui.resetGoodsCheck, SIGNAL(clicked()), this, SLOT(_ResetGoodsCheck()));

  model_transactions_view = new TransactionsViewModel(this);
  model_transactions_view->setTable("TransactionsView");

  QObject::connect(model_transactions_view, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(TransactionsDataChanged(const QModelIndex&, const QModelIndex&)));

  _InitTransactionView(ui.tableViewTransactions);  
  _InitTransactionView(ui.tableViewStatistic);  

  model_transactions = new QSqlTableModel(this);;
  model_transactions->setTable("Transactions");
  _ReloadModel(model_transactions);

  goods_check_view = new GoodsCheckModel(this);
  QObject::connect(goods_check_view, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(GoodsCheckDataChanged(const QModelIndex&, const QModelIndex&)));
  QObject::connect(model_goods, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(GoodsStoreDataChanged(const QModelIndex&, const QModelIndex&)));

  goods_check_view->setTable("GOODS_CHECK_VIEW");
  ui.tableViewGoodsCheck->setModel(goods_check_view);

  _ReloadModel(goods_check_view);

  _UpdateOrderPrice();
  _UpdatePurchasePrice();
  _UpdateCash();

  int num_users = model_users->rowCount();
  ui.lineEditPassword->setEchoMode(QLineEdit::Password);
  for(int i = 0; i < num_users; ++i)
    {
    QSqlRecord rec = model_users->record(i);
    int role = rec.value("ROLE_ID").toInt();
    if(role == UR_FAKE  || role > UR_BARTENDER)
      {
      int user_id = rec.value("USER_ID").toInt();
      ui.comboBoxResponsible->addItem(rec.value("LOGIN").toString(), user_id);
      }
    }

  ui.tableViewTransactions->scrollToBottom();
  ui.tableViewTransactions->resizeColumnsToContents();
  
  _SyncGoodsIcons(ui.listWidget, TransactionStatus::WaitingForPayment);
  ui.tableViewGoodsCheck->setSortingEnabled(true);
  _UpdateGoodsCheckMode();

  ui.tableViewGoodsCheck->resizeColumnsToContents();
    {
    auto verticalHeader = ui.tableViewGoodsCheck->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents); 
    }
  auto app = QBarApplication::instance();
  std::wstring str(L"Пользователь: ");
  QString user_str = QString::fromUtf16(str.c_str());
  ui.statusBar->showMessage(user_str + app->userName(app->currentUser()));

  depts_model = new QSqlQueryModel(this);
  _OnDebtsByPersons();

  if(app->currentUserRole() <= UR_BARTENDER)
    {
    ui.tabWidgetMain->removeTab(1);
    ui.tabWidgetMain->removeTab(1);
    ui.tabWidgetMain->removeTab(1);
    }

  ui.dateTimeEditStatTo->setDateTime(QDateTime::currentDateTime());
  }

//------------------------------------------------------------
bar::~bar()
  {
  }

//------------------------------------------------------------
void bar::OnDoubleClick(QModelIndex idx)
  {
  int row = idx.row();
  QSqlRecord rec = model_goods->record(row);
  int good_id = rec.value(0).toInt();
  int good_quantity = rec.value(3).toFloat();

  bool sale = ui.tabWidgetModes->currentIndex() == 0;
  bool purchasing = ui.tabWidgetModes->currentIndex() == 1;
  bool goods_withdraw = ui.tabWidgetModes->currentIndex() == 2;

  TransactionStatus::ETrStatus tr_status(TransactionStatus::Unknown);

  if(sale || goods_withdraw)
    {
    if(good_quantity <= 0)
      {
      QTimer::singleShot(150, this, SLOT(_NotifyNoGood()));
      return;
      }
    }

  if(sale)
    {
    tr_status =  TransactionStatus::WaitingForPayment;
    }
  else if(purchasing)
    {
    tr_status = TransactionStatus::WaitingForPurchasePayment;
    }
  else if(goods_withdraw)
    tr_status = TransactionStatus::PrepareGoodwithdrawal;

  if(tr_status != TransactionStatus::Unknown)
    {
    QDateTime curr = QDateTime::currentDateTime();
    QSqlRecord rec_tr = model_transactions->record();

    QString query_str = QString("SELECT TR_ID FROM Transactions WHERE GOOD_ID = %1 AND STATUS = %2").arg(good_id).arg(tr_status);
    QSqlQuery sql_qur;
    bool qres = sql_qur.exec(query_str);

    int tr_id(-1);

    if(sql_qur.next())
      tr_id = sql_qur.value(0).toInt();

    if(tr_id == -1)
      {
      float good_price = (purchasing||goods_withdraw) ? _getNewestPurchasePrice(good_id) : model_goods->GoodPrice(good_id);
      rec_tr.setValue(0, QVariant());  //TR_ID
      rec_tr.setValue(1, good_id);     //GOOD_ID
      rec_tr.setValue(2, tr_status);   //STATUS
      rec_tr.setValue(3, 1);           //Number
      rec_tr.setValue(4, good_price);  //Price
      rec_tr.setValue(5, QBarApplication::instance()->currentUserName()); //Comment
      rec_tr.setValue(6, curr);        //DATE
      model_transactions->insertRecord(-1, rec_tr);
      }
    else
      {

      QString query_str = QString("UPDATE Transactions SET Number=Number+1, DATE=\"%2\"  WHERE TR_ID = %1").arg(tr_id).arg(QDateTime::currentDateTime().toString(Qt::ISODate));
      if(false == _UpdateQuery(query_str))
        return;
      }

    _UpdateWhenTransactionsChanged();
    ui.tableViewTransactions->scrollToBottom();
    }
  }

//------------------------------------------------------------
void bar::OnOnConfirmSale()
  {
  auto change = ui.lineEditChange->text().toDouble();
  if(change < 0)
    {
    QMessageBox::warning(this, QString::fromUtf16(L"Невозможно продать товар"), QString::fromUtf16(L"Недостаточно денег внесено"));
    return;
    }

  QString query_str = "UPDATE Transactions\n"
                      "SET STATUS=1\n"
                      "WHERE STATUS=0;";

  if(false == _UpdateQuery(query_str))
    return;

  _ReloadModel(model_transactions);
  _ReloadModel(model_transactions_view);
  _ReloadModel(model_goods);
  _ReloadModel(goods_check_view);

  ui.tableViewTransactions->scrollToBottom();
  _UpdateOrderPrice();
  ui.doubleSpinBoxCustomerCash->setValue(0);
  _UpdateCash();

  ui.listWidget->clear();
  }

//------------------------------------------------------------
void bar::UpdateChange()
  {
  double customer_cash = ui.doubleSpinBoxCustomerCash->value();
  double to_pay = ui.lineEditOrderPrice->text().toDouble();
  double change = customer_cash - to_pay;

  ui.lineEditChange->setText(QString::number(change, 'f', 2));
  }

//------------------------------------------------------------
void bar::TransactionsDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
  {
  if(topLeft.isValid() && (topLeft == bottomRight))
    {
    int row = topLeft.row();
    int col = topLeft.column();

    double changed_value = topLeft.model()->data(topLeft).toDouble();
    QModelIndex status_id_idx = topLeft.model()->index(row, TransactionsView::Status);
    QModelIndex tr_id_idx =     topLeft.model()->index(row, TransactionsView::TransactionID);
    QModelIndex good_id_idx =   topLeft.model()->index(row, TransactionsView::GoodID);

    int good_id = good_id_idx.data().toInt();
    int tr_id = tr_id_idx.data().toInt();
    int status_id = status_id_idx.data().toInt();
    QString query_str;

    auto update_number = [&](int n)
      {
      query_str = QString("UPDATE Transactions SET Number=%1 WHERE TR_ID=%2").arg(n).arg(tr_id);
      };

    if(status_id == TransactionStatus::WaitingForPurchasePayment)
      {
      if(col == TransactionsViewColumn::Number)
        {
        int new_number = topLeft.data().toInt();
        int number = std::max(0, new_number);
        update_number(number);
        }
      if(col == TransactionsViewColumn::Price)
        query_str = QString("UPDATE Transactions SET Price=%1 WHERE TR_ID=%2").arg(changed_value, 0, 'f', 2).arg(tr_id);

      if(col == TransactionsViewColumn::Sum)
        {
        QModelIndex tr_id_idx = topLeft.model()->index(row, 2);
        double number = tr_id_idx.data().toDouble();

        double price = changed_value/number;
        query_str = QString("UPDATE Transactions SET Price=%1 WHERE TR_ID=%2").arg(price, 0, 'f', 2).arg(tr_id);
        }
      }

    if(status_id == TransactionStatus::WaitingForPayment)
      {
      int number = std::min((int)changed_value, model_goods->GoodNumber(good_id));
      if(col == TransactionsViewColumn::Number)
        update_number(number);
      }

    if(false == _UpdateQuery(query_str))
      return;

    _ReloadModel(model_transactions_view);
    _ReloadModel(model_transactions);
    _ReloadModel(model_goods);
    ui.tableViewTransactions->scrollToBottom();
    _UpdateOrderPrice();
    _UpdatePurchasePrice();

    _SyncGoodsIcons(ui.listWidget, TransactionStatus::WaitingForPayment);
    }
  }

//------------------------------------------------------------
void bar::OnOnConfirmPurchase()
  {
  double cash = ui.lineEditCash->text().toDouble();
  double to_pay = ui.lineEditPurchasePrice->text().toDouble();

  if(to_pay > cash)
    {
    QMessageBox::warning(this, QString::fromUtf16(L"Ошибка выволнения"), QString::fromUtf16(L"Недасточно денег в кассе пароль"));
    return;
    }

  QString query_str_update_status = QString("UPDATE Transactions SET Price=-Price WHERE STATUS=%1").arg(TransactionStatus::WaitingForPurchasePayment);
  QString query_str_update_price = QString("UPDATE Transactions SET STATUS=%1 WHERE STATUS=%2").arg(TransactionStatus::Bought).arg(TransactionStatus::WaitingForPurchasePayment);

  if(false == _UpdateQuery(query_str_update_status))
    return;

  if(false == _UpdateQuery(query_str_update_price))
    return;

  _ReloadModel(model_transactions_view);
  _ReloadModel(model_transactions);
  _ReloadModel(model_goods);

  _UpdateOrderPrice();
  ui.doubleSpinBoxCustomerCash->setValue(0);
  _UpdatePurchasePrice();
  _UpdateCash();
  ui.tableViewTransactions->scrollToBottom();
  }

//---------------------------------------------------------------------------------------------------
void bar::_UpdatePurchasePrice()
  {
  model_purchase_price->query().exec();
  QString str = QString::number(model_purchase_price->record(0).value(0).toDouble(), 'f', 2);
  ui.lineEditPurchasePrice->setText(str);
  }

//---------------------------------------------------------------------------------------------------
void bar::_UpdateCash()
  {
  model_cash->query().exec();
  QString str = QString::number(model_cash->record(0).value(0).toDouble(), 'f', 2);
  ui.lineEditCash->setText(str);
  }

//---------------------------------------------------------------------------------------------------
void bar::_UpdateOrderPrice()
  {
  model_order_price->query().exec();
  QString str = QString::number(model_order_price->record(0).value(0).toDouble(), 'f', 2);
  ui.lineEditOrderPrice->setText(str);
  }

//---------------------------------------------------------------------------------------------------
void bar::OnOnChangeCash()
  {
  int user_id = ui.comboBoxResponsible->currentData().toInt();
  QString pass = QBarApplication::instance()->userPassword(user_id);
  
  if(ui.lineEditPassword->text() ==  pass)
    {
    bool cash_mode = ui.radioButtonCash->isChecked();

    if(cash_mode)
      {
      double price = ui.doubleSpinBoxIncomeOutcome->value();
      QDateTime curr = QDateTime::currentDateTime();
      QSqlRecord rec_tr = model_transactions->record();

      rec_tr.setValue(0, QVariant());  //TR_ID
      rec_tr.setValue(1, 9999);       //GOOD_ID
      rec_tr.setValue(2, 3);          //STATUS
      rec_tr.setValue(3, 1);            //Number
      rec_tr.setValue(4, price);       //Price
      rec_tr.setValue(5, ui.comboBoxResponsible->currentText()+": "+ui.lineEditReason->text()); //Comment
      rec_tr.setValue(6, curr);        //DATE

      int rc = model_transactions->rowCount();
      bool res = model_transactions->insertRecord(-1, rec_tr);
      }
    else
      {
      QString query_str = QString("SELECT TR_ID FROM Transactions WHERE STATUS=%1").arg(TransactionStatus::PrepareGoodwithdrawal);
      QSqlQuery query(query_str);
      QVector<int> ids;
      while(query.next())
        ids.push_back(query.value("TR_ID").toInt());

      query_str = QString("UPDATE Transactions SET STATUS=%1, COMMENT=\"%2\" WHERE STATUS=%3").
        arg(TransactionStatus::Goodwithdrawal).arg(ui.comboBoxResponsible->currentText()).arg(TransactionStatus::PrepareGoodwithdrawal);

      if(false ==_UpdateQuery(query_str))
        return;

      for(int i = 0; i < ids.size(); ++i)
        {
        query_str = QString("INSERT INTO GoodWithhold Values(%1, %2)").arg(ids[i]).arg(user_id);
        QSqlQuery q(query_str);
        }
      ui.listWidget->clear();
      }

    _ReloadModel(model_transactions);
    _ReloadModel(model_transactions_view);

    Q_ASSERT(model_transactions_view->rowCount() == model_transactions->rowCount());

    _ReloadModel(model_goods);
    _UpdateCash();

    ui.lineEditPassword->clear();
    ui.tableViewTransactions->scrollToBottom();
    ui.lineEditReason->clear();
    }
  else
    {
    QPoint pos = QCursor::pos();
    QToolTip::showText(pos, QString::fromUtf16(L"Неправильный пароль"), 0, QRect(), 1000);
    }
  }

//------------------------------------------------------
void bar::_ReloadModel( QSqlTableModel* ip_model)
  {
  ip_model->select();
  while(ip_model->canFetchMore())
    {
    ip_model->fetchMore();
    }
  }

//------------------------------------------------------
double bar::_getNewestPurchasePrice( int id )
  {
  QString query_str = QString("SELECT Price, MAX(Date) FROM Transactions WHERE GOOD_ID = %1 and STATUS = %2 AND PRICE <> 0").arg(id).arg(TransactionStatus::Bought);
  QSqlQuery sql_qur;
  bool qres = sql_qur.exec(query_str);
  sql_qur.next();
  QSqlRecord rec = sql_qur.record();
  double price = rec.value(0).toDouble();
  Q_ASSERT(price <= 0);
  
  if(price == 0)
    {
    price = - model_goods->GoodPrice(id) / 2; 
    }

  return -price;
  }

//---------------------------------------------------------------------------------------------------
QListWidgetItem* bar::_findGoodItem( int good_id )
  {
  for(int i(0); i < ui.listWidget->count(); ++i)
    {
    auto item = ui.listWidget->item(i); 
    if(item->data(9999).toInt() == good_id)
      {
      return item;
      }
    }
  return nullptr;
  }

//---------------------------------------------------------------------------------------------------
void bar::_SyncGoodsIcons(QListWidget* ip_list, TransactionStatus::ETrStatus i_tr_status)
  {
  QString query_str = QString("SELECT GOOD_ID, Number FROM Transactions WHERE STATUS = %1").arg(i_tr_status);
  QSqlQuery sql_qur(query_str);
  sql_qur.exec();

  ip_list->clear();
  while(sql_qur.next())
    {
    int good_id = sql_qur.value(0).toInt();
    int good_number = sql_qur.value(1).toInt();
    if(_findGoodItem(good_id) == nullptr)
      {
      auto p_item(new QListWidgetItem(ip_list));
      QString str;
      str+= model_goods->GoodName(good_id) + "(";
      auto app = QBarApplication::instance();

      p_item->setIcon(app->goodIcon(good_id));
      p_item->setText(str + QString::number(good_number) + QString(")"));
      p_item->setData(9999, good_id);
      ui.listWidget->addItem(p_item);
      }
    else
      {
      auto item = _findGoodItem(good_id);
      item->setText(QString::number(good_number));
      }
    }

  }

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
void bar::_UpdateTakeOffMode()
  {
  bool cash_mode = ui.radioButtonCash->isChecked();

  ui.doubleSpinBoxIncomeOutcome->setHidden(!cash_mode);
  ui.lineEditReason->setHidden(!cash_mode);
  ui.labelReason->setHidden(!cash_mode);
  }

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
void bar::_PerformGoodsCheck()
  {
  auto model = ui.tableViewGoodsCheck->model();
  int num_rows = model->rowCount();

  TransactionStatus::ETrStatus status;

  QString query_str("INSERT INTO TRANSACTIONS VALUES");
  for(int i = 0; i <num_rows; ++i)
    {
    int good_id = model->data(model->index(i, 0)).toInt();
    int diff = model->data(model->index(i, 3)).toInt();
    
    status = (diff < 0) ? TransactionStatus::Sold : ( (diff > 0) ? TransactionStatus::SaleCanceled : TransactionStatus::Unknown);

    if(status != TransactionStatus::Unknown)
      {
      float price = model_goods->GoodPrice(good_id);
      if(diff > 0)
        price = -price;
      query_str += QString::fromUtf16(std::wstring(L"(null, %1, %2, %3, %4, \"инвент\", datetime('now', 'localtime')),").c_str()).arg(good_id).arg(status).arg(std::abs(diff)).arg(price);
      }

    }

  query_str.resize(query_str.size() - 1);
  bool res = QSqlQuery(query_str).result();

  Q_ASSERT(res);

  _ReloadModel(model_transactions);
  _ReloadModel(model_transactions_view);
  _ReloadModel(model_goods);
  _ReloadModel(goods_check_view);
  _UpdateCash();
  _UpdateGoodsCheckMode();
  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::MainTabWidgetChanged( int idx)
  {
  if(idx == 2)
    {
    _OnDebtsByPersons();
    }
  
  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::_UpdateGoodsCheckMode()
  {
  QSqlQuery sql_qur("select  * from GoodsCheckView");

  if(sql_qur.next())
    {
    ui.lineEditGoodPlus->setText(sql_qur.value(0).toString());
    ui.lineEditGoodMinus->setText(sql_qur.value(1).toString());
    }
  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::GoodsCheckDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
  {
  _UpdateGoodsCheckMode();
  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::_ResetGoodsCheck()
  {
  if(false == _UpdateQuery("DELETE FROM GOODS_CHECK"))
    return;

  _ReloadModel(goods_check_view);
  _UpdateGoodsCheckMode();
  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::_NotifyNoGood()
  {
  QPoint pos = QCursor::pos();
  QToolTip::showText(pos, QString::fromUtf16(L"Товара нету на складе"), 0, QRect(), 500);
  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::_OnRightClickGoodsView(QPoint pos)
  {
  auto app = QBarApplication::instance();
  QMenu* menu = new QMenu(this);
  int role = app->currentUserRole();
  auto table = ui.tableViewGood;
  QModelIndex index=table->indexAt(pos);

  if(app->currentUserRole() > UR_BARTENDER)
    {
    menu->addAction(ui.actionAddGood);
    menu->addAction(ui.actionRemoveGood);
    menu->addAction(ui.actionRemoveGoodCompletly);
    }

  if(role <= UR_BARTENDER || role > UR_BARTENDER)
    {
    menu->addAction(ui.actionFilterGood);
    menu->addAction(ui.actionResetGoodFilter);
    }

  if(menu->actions().empty() == false)
    {
    menu->popup(table->viewport()->mapToGlobal(pos));
    }
  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::_OnAddGood()
  {
  QString query_str = (QString::fromUtf16(L"INSERT INTO GOODS VALUES(NULL, \"Новый товар\", 0)"));
  if(false == _UpdateQuery(query_str))
    return;

  _ReloadModel(model_goods);
  auto table = ui.tableViewGood;
  QModelIndex idx = table->model()->index(0,1);
  QModelIndexList list = table->model()->match(idx, Qt::DisplayRole, QString::fromUtf16(L"Новый товар"));
  table->scrollTo(list.first());
  table->selectRow(list.first().row());
  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::_OnRemoveGood()
  {
  auto table = ui.tableViewGood;
  QModelIndex idx = table->currentIndex();
  if(idx.isValid())
    {
    int good_id = model_goods->record(idx.row()).value("ID").toInt();
    QString query_str = QString("INSERT INTO REMOVED_GOODS VALUES(%1)").arg(good_id);
    if(_UpdateQuery(query_str))
      _ReloadModel(model_goods);
    }
  }

//----------------------------------------------------------------------------------------------------------
void bar::_OnRemoveGoodCompletely()
  {
  QMessageBox box(QMessageBox::Question, QString::fromUtf16(L"Полное удаление товара"), 
    QString::fromUtf16(L"Все зависимые транзакции также удлятся. Продолжить?"), QMessageBox::Yes | QMessageBox::No);

  if(box.exec() == QMessageBox::Yes)
    {
    QString delete_transactions = "DELETE FROM GOODS WHERE ";
    const QString or(" OR "); 

    auto table = ui.tableViewGood;
    auto indices = table->selectionModel()->selectedIndexes();

    QSet<int> rows;
    for(auto i = indices.begin(); i != indices.end(); ++i)
      rows.insert(i->row());

    for(auto i = rows.begin(); i != rows.end(); ++i)
      {
      auto data = table->model()->index(*i, GoodsStoreView::ID).data();

      delete_transactions += "GOOD_ID = " + data.toString() + " OR ";
      }

    delete_transactions.resize(delete_transactions.size() - or.size());

    if(!indices.empty())
      {
      if(_UpdateQuery(delete_transactions))
        {
        _UpdateWhenTransactionsChanged();
        }
      }
    }
  }


//-----------------------------------------------------------------------------------------------------------------------------
void bar::_OnAddGoodFilter()
  {
  auto table = ui.tableViewGood;
  QModelIndex idx = table->currentIndex();
  if(idx.isValid())
    {
    int good_id = model_goods->record(idx.row()).value("ID").toInt();
    QString filter = model_transactions_view->filter();
    if(!filter.isEmpty())
      filter+=" OR ";

    filter+="GOOD_ID=" + QString::number(good_id);
    model_transactions_view->setFilter(filter);
    }
  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::_OnResetGoodFilter()
  {
  model_transactions_view->setFilter("");
  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::GoodsStoreDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
  {
  if(topLeft.isValid() && topLeft == bottomRight)
    {
    int row = topLeft.row();
    int col = topLeft.column();

    QString quary_str;

    if(col == 1)
      {
      int good_id = model_goods->record(row).value("ID").toInt();

      QString new_good_name = topLeft.data().toString();
      quary_str = (QString("UPDATE GOODS SET Name=\"%1\" WHERE GOOD_ID=%2").arg(new_good_name).arg(good_id));
      }

    if(col == 2)
      {
      int good_id = model_goods->record(row).value("ID").toInt();

      float new_price = topLeft.data().toFloat();
      quary_str = (QString("UPDATE GOODS SET Price=\"%1\" WHERE GOOD_ID=%2").arg(new_price).arg(good_id));
      }


    if(false == _UpdateQuery(quary_str))
      return;
    }
  }

//----------------------------------------------------------------------------------------
void bar::_OnDebtsByPersons()
  {
  depts_model->setQuery("SELECT * FROM GoodsDebtByPersons");
  ui.tableViewDepts->setModel(depts_model);
  ui.tableViewDepts->hideColumn(0);
  }

//----------------------------------------------------------------------------------------
void bar::_OnDebtsByGood()
  {
  depts_model->setQuery("SELECT * FROM GoodsDebt");
  ui.tableViewDepts->setModel(depts_model);
  ui.tableViewDepts->hideColumn(0);
  }

//----------------------------------------------------------------------------------------
void bar::_OnRemoveDebt()
  {
  bool is_by_persons = ui.radioButtonDebtsByUser->isChecked();
  QModelIndex idx = ui.tableViewDepts->currentIndex();

  if(idx.isValid())
    {
    int row = idx.row();
    auto rec = depts_model->record(row);
    int user_id = rec.value(0).toInt();
    
    QString query_str = QString("UPDATE Transactions SET STATUS=%1 WHERE TR_ID IN (SELECT TR_ID FROM GoodWithhold WHERE USER_ID = %2)").arg(TransactionStatus::PaydedWithDiscount).arg(user_id);

    if(false == _UpdateQuery(query_str))
      return;

    _OnDebtsByPersons();
    _UpdateCash();
    _ReloadModel(model_transactions_view);
    }
  }

//----------------------------------------------------------------------------------------------------------
void bar::_InitTransactionView( QTableView* ip_tblv)
  {
  ip_tblv->setModel(model_transactions_view);
  _ReloadModel(model_transactions_view);

  ip_tblv->hideColumn(0);
  ip_tblv->hideColumn(7);
  ip_tblv->hideColumn(8);
  ip_tblv->resizeColumnsToContents();
  auto verticalHeader = ip_tblv->verticalHeader();
  verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents); 
  }

//----------------------------------------------------------------------------------------------------------
void bar::_UpdateStat()
  {
  QString query_str = "SELECT SUM(Transactions.[NUMBER] * Transactions.[PRICE])\n"
                      "FROM Transactions\n"
                      " WHERE ";

  std::map<QCheckBox*, TransactionStatus::ETrStatus> line_edit_status;

  line_edit_status[ui.checkBoxSold] = TransactionStatus::Sold;
  line_edit_status[ui.checkBoxBought] = TransactionStatus::Bought;
  line_edit_status[ui.checkBoxCancelOrder] = TransactionStatus::SaleCanceled;
  line_edit_status[ui.checkBoxCost] = TransactionStatus::PaydedWithDiscount;
  line_edit_status[ui.checkBoxWithDraw] = TransactionStatus::Goodwithdrawal;

  auto end = line_edit_status.end();

  QString condition;

  QString condition_tr = "(";
  for(auto i  = line_edit_status.begin(); i != end; ++i)
    {
    if(i->first->isChecked())
      {
      condition_tr += QString("Transactions.[STATUS]=%1 OR ").arg(i->second);
      }
    }

  if(condition_tr.length() > 1)
    {
    condition_tr.resize(condition_tr.size() - 4);
    condition_tr+=") AND ";
    condition += condition_tr;
    }

  if(ui.checkBoxWrittenOff->isChecked())
    {
    query_str += "(Transactions.[TR_ID] IN (SELECT GoodWithhold.[TR_ID] FROM GoodWithhold WHERE USER_ID = 7)) AND ";
    }

  auto date_to_str = [](QDateTimeEdit* ip_date)
    {
    return ip_date->dateTime().toString(Qt::ISODate);
    };

  auto date_from = date_to_str(ui.dateTimeEditStatFrom);
  auto date_to = date_to_str(ui.dateTimeEditStatTo);
  
  condition += QString("Transactions.[DATE] BETWEEN \"%1\" AND \"%2\" ").arg(date_from).arg(date_to);

  query_str += condition;

  QSqlQuery query(query_str);
  
  float value(0);

  if(query.next())
    value = query.value(0).toFloat();
  ui.lineEditStatSum->setText(QString::number(value, 'f', 2));
  }

//----------------------------------------------------------------------------------------------------------
bool bar::_UpdateQuery(const QString& i_query_str)
  {
  if(i_query_str.isEmpty())
    return true;

  QSqlQuery sql_qur(i_query_str);

  auto error = sql_qur.lastError();
  if(error.isValid())
    {
    QMessageBox::warning(this, QString::fromUtf16(L"Ошибка запроса"), error.text());
    return false;
    }

  return true;
  }

//----------------------------------------------------------------------------------------------------------
void bar::_TransactionsViewRightClick(QPoint pos)
  {
  auto table = ui.tableViewTransactions;
  QModelIndex index=table->indexAt(pos);
  QMenu *menu=new QMenu(this);
  menu->addAction(ui.actionRemoveTransaction);
  menu->popup(table->viewport()->mapToGlobal(pos));
  }

//----------------------------------------------------------------------------------------------------------
void bar::_OnRemoveTransaction()
  {
  QString delete_transactions = "DELETE FROM TRANSACTIONS WHERE ";
  const QString or(" OR "); 

  auto table = ui.tableViewTransactions;
  auto indices = table->selectionModel()->selectedIndexes();

  QSet<int> rows;
  for(auto i = indices.begin(); i != indices.end(); ++i)
    rows.insert(i->row());

  for(auto i = rows.begin(); i != rows.end(); ++i)
    {
    auto data = table->model()->index(*i,TransactionsView::TransactionID).data();

    delete_transactions += "TR_ID = " + data.toString() + " OR ";
    }

  delete_transactions.resize(delete_transactions.size() - or.size());

  if(!indices.empty())
    {
    if(_UpdateQuery(delete_transactions))
      {
      _UpdateWhenTransactionsChanged();
      }
    }
  }

//----------------------------------------------------------------------------------------------------------
void bar::_UpdateWhenTransactionsChanged()
  {
  _SyncGoodsIcons(ui.listWidget, TransactionStatus::WaitingForPayment);
  _ReloadModel(model_transactions);
  _ReloadModel(model_transactions_view);
  _ReloadModel(model_goods);

  _UpdateOrderPrice();
  _UpdatePurchasePrice();
  _UpdateCash();
  }

//------------------------------------------------------------
void bar::_IncFont()
  {
  int cFont = _fontSize();

  if(cFont != -1)
    {
    ++cFont;
    _setFont(cFont);
    }
  }

//------------------------------------------------------------
void bar::_DecFont()
  {
  int cFont = _fontSize();

  if(cFont != -1)
    {
    --cFont;
    _setFont(cFont);
    }
  }

//------------------------------------------------------------
int bar::_fontSize()
  {
  QString style = styleSheet();
  int idx = style.indexOf(QRegularExpression("font: .*pt"));

  if(idx != -1)
    {
    int number_end = style.indexOf("pt",idx);
    QString number = style.mid(idx+6, number_end - idx-6);
    return number.toInt();
    }

  return 8;
  }

//------------------------------------------------------------
void bar::_setFont(int new_font)
  {
  auto core_app = QBarApplication::instance();

  QString stl = styleSheet();
  stl.replace(QRegularExpression("font: .*pt"), QString("font: ") + QString::number(new_font) + "pt");
  setStyleSheet(stl);

  int font = core_app->settings().value("UI/font", 8).toInt();
  if(font != new_font)
     core_app->settings().setValue("UI/font", new_font);
  }

//------------------------------------------------------------
void bar::_OnExit()
  {
  QBarApplication::instance()->exit(10);
  }

//------------------------------------------------------------
void bar::_GoodsFilterChanged()
  {
  QString f_str = ui.lineEditGoodsFilter->text();

  if(f_str.isEmpty() == false)
    {
    QString filt = QString("*") + f_str + QString("*");

    auto proxy_model = new QSortFilterProxyModel();
    proxy_model->setSourceModel(model_goods);
    proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy_model->setFilterKeyColumn(1);
    proxy_model->setFilterWildcard(filt);

    ui.tableViewGood->setModel(proxy_model);

    }
  else
    {
    ui.tableViewGood->setModel(model_goods);
    }
  }


