#include "bar.h"
#include <QtSql\qsqldatabase.h>
#include <QtSql\qsqltablemodel.h>
#include <QtSql\qsqlrecord.h>
#include <QtSql\qsqlquery.h>
#include <QtCore\qdatetime.h>
#include <QtSql\qsqlerror.h>
#include <QtSql\QSqlRelationalTableModel.h>
#include <QtWidgets\qscrollbar.h>
#include <QtWidgets\qtooltip.h>
#include <QtCore\QTimer>
#include <QtWidgets\QMessageBox>
#include <QStyledItemDelegate>
#include "Models\GoodsStoreModel.h"
#include "TransactionStatus.h"
#include "Models\TransactionsViewModel.h"
#include "Models\GoodsCheckModel.h"
#include <QtSql\qsqlfield.h>
#include <QtSql\qsqlindex.h>
#include <QtCore\qthread.h>
#include "QLoginDialog.h"
#include "QBarApplication.h"
#include <QtWidgets\qheaderview.h>
#include <QtCore\qstandardpaths.h>

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


//------------------------------------------------------------
bar::bar(QWidget *parent)
    : QMainWindow(parent)
  {
  model_users = QBarApplication::instance()->model_users();

  ui.setupUi(this);
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
  ui.tableViewGood->setIconSize(QSize(50,50));

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

  ui.tableViewTransactions->setModel(model_transactions_view);
  
  ui.tableViewTransactions->hideColumn(0);
  ui.tableViewTransactions->hideColumn(7);
  ui.tableViewTransactions->hideColumn(8);
  _ReloadModel(model_transactions_view);
  ui.tableViewTransactions->resizeColumnsToContents();

  model_transactions = new QSqlTableModel(this);;
  model_transactions->setTable("Transactions");
  _ReloadModel(model_transactions);

  goods_check_view = new GoodsCheckModel(this);
  QObject::connect(goods_check_view, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(GoodsCheckDataChanged(const QModelIndex&, const QModelIndex&)));
  QObject::connect(model_goods, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(GoodsStoreDataChanged(const QModelIndex&, const QModelIndex&)));

  goods_check_view->setTable("GOODS_CHECK_VIEW");
  ui.tableViewGoodsCheck->setModel(goods_check_view);

  _ResetGoodsCheck();
  _ReloadModel(goods_check_view);

  _UpdateOrderPrice();
  _UpdatePurchasePrice();
  _UpdateCash();

  int num_users = model_users->rowCount();
  ui.lineEditPassword->setEchoMode(QLineEdit::Password);
  for(int i = 0; i < num_users; ++i)
    {
    QSqlRecord rec = model_users->record(i);
    if(rec.value("ROLE_ID").toInt()  == 0 || rec.value("ROLE_ID").toInt() == 20)
      {
      int user_id = rec.value("USER_ID").toInt();
      ui.comboBoxResponsible->addItem(rec.value("LOGIN").toString(), user_id);
      }
    }

  ui.tableViewTransactions->scrollToBottom();
  ui.tableViewTransactions->resizeColumnsToContents();
  _SyncGoodsIcons();
  ui.tableViewGoodsCheck->setSortingEnabled(true);
  _UpdateGoodsCheckMode();

  auto app = QBarApplication::instance();
  std::wstring str(L"Пользователь: ");
  QString user_str = QString::fromUtf16(str.c_str());
  ui.statusBar->showMessage(user_str + app->userName(app->currentUser()));

  if(app->currentUserRole() == 10)
    {
    ui.tabWidgetMain->removeTab(1);
    ui.tabWidgetMain->removeTab(1);
    }
  depts_model = new QSqlQueryModel(this);
  _OnDebtsByPersons();

  ui.dateTimeEditBegin->setDateTime(QDateTime::currentDateTime().addDays(-7));
  ui.dateTimeEditEnd->setDateTime(QDateTime::currentDateTime());
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

    int count = sql_qur.record().count();
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
      QString query_str = QString("UPDATE Transactions SET Number=Number+1, DATE=datetime('now', 'localtime')  WHERE TR_ID = %1").arg(tr_id);
      QSqlQuery sql_qur(query_str);
      }

    _SyncGoodsIcons();
    _ReloadModel(model_transactions);
    _ReloadModel(model_transactions_view);
    _ReloadModel(model_goods);

    _UpdateOrderPrice();
    _UpdatePurchasePrice();

    ui.tableViewTransactions->scrollToBottom();


    }
  }

//------------------------------------------------------------
void bar::OnOnConfirmSale()
  {
  auto change = ui.lineEditChange->text().toDouble();
  if(change < 0)
    {
    QMessageBox::warning(this, "Невозможно продать товар", "Недостаточно денег внесено");
    return;
    }

  QString query_str = "UPDATE Transactions\n"
                      "SET STATUS=1\n"
                      "WHERE STATUS=0;";

  QSqlQuery sql_qur;
  bool res = sql_qur.exec(query_str);
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

    QSqlQuery sql_qur;
    sql_qur.exec(query_str);


    _ReloadModel(model_transactions_view);
    _ReloadModel(model_transactions);
    _ReloadModel(model_goods);
    ui.tableViewTransactions->scrollToBottom();
    _UpdateOrderPrice();
    _UpdatePurchasePrice();

    _SyncGoodsIcons();
    }
  }

//------------------------------------------------------------
void bar::OnOnConfirmPurchase()
  {
  double cash = ui.lineEditCash->text().toDouble();
  double to_pay = ui.lineEditPurchasePrice->text().toDouble();


  if(to_pay > cash)
    {
    QMessageBox::warning(this, "Ошибка выволнения", "Недасточно денег в кассе пароль");
    return;
    }

  QString query_str_update_status = QString("UPDATE Transactions SET Price=-Price WHERE STATUS=%1").arg(TransactionStatus::WaitingForPurchasePayment);
  QString query_str_update_price = QString("UPDATE Transactions SET STATUS=%1 WHERE STATUS=%2").arg(TransactionStatus::PurchasePayed).arg(TransactionStatus::WaitingForPurchasePayment);

  QSqlQuery sql_qur;

  bool res = sql_qur.exec(query_str_update_status);
  Q_ASSERT(res);

  res = sql_qur.exec(query_str_update_price);
  Q_ASSERT(res);

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
      rec_tr.setValue(5, ui.comboBoxResponsible->currentText()); //Comment
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

      QSqlQuery q(query_str);

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
  QString query_str = QString("SELECT Price, MAX(Date) FROM Transactions WHERE GOOD_ID = %1 and STATUS = %2 AND PRICE <> 0").arg(id).arg(TransactionStatus::PurchasePayed);
  QSqlQuery sql_qur;
  bool qres = sql_qur.exec(query_str);
  sql_qur.next();
  QSqlRecord rec = sql_qur.record();
  double price = rec.value(0).toDouble();
  Q_ASSERT(price <= 0);
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
void bar::_SyncGoodsIcons()
  {
  QString query_str = QString("SELECT GOOD_ID, Number FROM Transactions WHERE STATUS = %1").arg(TransactionStatus::WaitingForPayment);
  QSqlQuery sql_qur(query_str);
  sql_qur.exec();

  ui.listWidget->clear();
  while(sql_qur.next())
    {
    int good_id = sql_qur.value(0).toInt();
    int good_number = sql_qur.value(1).toInt();
    if(_findGoodItem(good_id) == nullptr)
      {
      auto p_item(new QListWidgetItem(ui.listWidget));
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
  if(ui.radioButtonCash->isChecked())
    {
    ui.doubleSpinBoxIncomeOutcome->show();
    }
  else
    {
    ui.doubleSpinBoxIncomeOutcome->hide();
    }
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
    
    status = (diff < 0) ? TransactionStatus::Payed : ( (diff > 0) ? TransactionStatus::SaleCanceled : TransactionStatus::Unknown);

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
  QSqlQuery sql_qur("DELETE FROM GOODS_CHECK");
  QString query_str("INSERT INTO GOODS_CHECK VALUES");
  for(int i = 0; i <model_goods->rowCount(); ++i)
    {
    auto rec = model_goods->record(i);
    query_str += QString("(%1, %2),").arg(rec.value(0).toInt()).arg(rec.value(3).toInt());
    }

  query_str.resize(query_str.size() - 1);
  bool res = QSqlQuery(query_str).result();

  Q_ASSERT(res);

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
  auto table = ui.tableViewGood;
  QModelIndex index=table->indexAt(pos);
  QMenu *menu=new QMenu(this);
  menu->addAction(ui.actionAddGood);
  menu->addAction(ui.actionRemoveGood);
  menu->popup(table->viewport()->mapToGlobal(pos));
  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::_OnAddGood()
  {
  QString query_str = (QString::fromUtf16(L"INSERT INTO GOODS VALUES(NULL, \"Новый товар\", 0)"));
  QSqlQuery sql_qur(query_str);
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
    QSqlQuery sql_qur(query_str);
    _ReloadModel(model_goods);
    }
  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::_OnEditGood()
  {

  }

//-----------------------------------------------------------------------------------------------------------------------------
void bar::GoodsStoreDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
  {
  if(topLeft.isValid() && topLeft == bottomRight)
    {
    int row = topLeft.row();
    int col = topLeft.column();

    if(col == 1)
      {
      int good_id =   model_goods->record(row).value("ID").toInt();

      QString new_good_name = topLeft.data().toString();
      QString quary_str(QString("UPDATE GOODS SET Name=\"%1\" WHERE GOOD_ID=%2").arg(new_good_name).arg(good_id));
      bool res = QSqlQuery().exec(quary_str);
      Q_ASSERT(res);
      }

    if(col == 2)
      {
      int good_id = model_goods->record(row).value("ID").toInt();

      float new_price = topLeft.data().toFloat();
      QString quary_str(QString("UPDATE GOODS SET Price=\"%1\" WHERE GOOD_ID=%2").arg(new_price).arg(good_id));
      bool res = QSqlQuery().exec(quary_str);
      Q_ASSERT(res);
      }
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
    QSqlQuery sql_qur(query_str);
    _OnDebtsByPersons();
    _UpdateCash();
    }
  }

//----------------------------------------------------------------------------------------
void bar::_OnUpdateTotal()
  {
  const wchar_t* psz_query = L"SELECT SUM(Transactions.[NUMBER] * Transactions.[PRICE])\n"
                             L"FROM Transactions\n"
                             L"WHERE (Transactions.[STATUS] = 1 /*Продано*/\n"
                             L"OR Transactions.[STATUS] = 8) /*Отмена продажи*/\n"
                             L"AND Transactions.[DATE] BETWEEN \"%1\" AND \"%2\"";

  auto query_str = QString::fromUtf16(psz_query).arg(ui.dateTimeEditBegin->text()).arg(ui.dateTimeEditEnd->text());

  QSqlQuery query(query_str);
  float value(0);

  if(query.next())
    value = query.value(0).toFloat();

  ui.lineEditTotal->setText(QString::number(value, 'f', 2) + QString::fromUtf16(L" грн."));
  }
