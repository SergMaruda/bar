#include "bar.h"
#include <QtSql\qsqldatabase.h>
#include <QtSql\qsqltablemodel.h>
#include <QtSql\qsqlrecord.h>
#include <QtSql\qsqlquery.h>
#include <QtCore\qdatetime.h>
#include <QtSql\qsqlerror.h>
#include <QtWidgets\qscrollbar.h>
#include <QtWidgets\qtooltip.h>
#include <QtCore\QTimer>
#include <QtWidgets\QMessageBox>
#include <QStyledItemDelegate>
#include "Models\GoodsStoreModel.h"

namespace Column
  {
  enum EColumns
    {
    Number = 2,
    Price = 3,
    Sum = 4
    };
  };

 
namespace TransactionsView
  {
  enum
    {
    Status = 0,
    TransactionID = 7,
    GoodID = 8
    };
  }


namespace TransactionStatus
  {
  enum ETrStatus
    {
    WaitingForPayment = 0,
    Payed = 1,
    WaitingForPurchasePayment = 2,
    PurchasePayed = 3,
    Canceled = 4,
    Goodwithdrawal = 5,
    GoodPayedCostPrice = 6,
    NotUsed = 7
    };
  };

class QSqlTableModelMy: public QSqlTableModel
  {
  public:
  explicit QSqlTableModelMy(QObject *parent = 0, QSqlDatabase db = QSqlDatabase()):
  QSqlTableModel(parent, db)
    {
    }


  Qt::ItemFlags flags(const QModelIndex &index) const
    {
    Qt::ItemFlags f = QSqlTableModel::flags(index);
    int row = index.row();

    if(record(row).value(0).toInt() == TransactionStatus::WaitingForPurchasePayment && (index.column() == Column::Number || index.column() == Column::Price || index.column() == Column::Sum))
      return Qt::ItemFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

    if(record(row).value(0).toInt() == TransactionStatus::WaitingForPayment && (index.column() == Column::Number))
      return Qt::ItemFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

    return  Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

  QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const
    {
    if (role == Qt::CheckStateRole)
      return QVariant();

    if (role == Qt::BackgroundRole) 
      {
      int row = idx.row();
 
      QSqlRecord rec = record(row);
      int tr_status = rec.value(0).toInt();

      if(tr_status == TransactionStatus::WaitingForPayment)
        {
        QColor color(255,0,0, 128);           
        return QBrush(color);
        }
      if(tr_status == TransactionStatus::Payed)
        {
        QColor color(0,255,0, 128);           
        return QBrush(color);
        }
      if(tr_status == TransactionStatus::WaitingForPurchasePayment)
        {
        QColor color(255,0,0, 64);           
        return QBrush(color);
        }
      if(tr_status == TransactionStatus::PurchasePayed)
        {
        QColor color(0,255,0, 64);            
        return QBrush(color);
        }

      }
    return QSqlTableModel::data(idx, role);
    }
  };


//------------------------------------------------------------
bar::bar(QWidget *parent)
    : QMainWindow(parent)
  {
  ui.setupUi(this);
  ui.tableViewGood->hideColumn(1);

   
  m_db = QSqlDatabase::addDatabase("QSQLITE");
  m_db.setDatabaseName("..\\Database\\bar.db");
  bool ok = m_db.open();
  if(!ok)
    {
    m_db.setDatabaseName("..\\..\\..\\Database\\bar.db");
    ok = m_db.open();
    }

  if(!ok)
    {
    m_db.setDatabaseName("bar.db");
    ok = m_db.open();
    }

  if(!ok)
    {
    QMessageBox::critical(this, "Ошибка открытия базы данных", "Невозможно открыть файл базы данных.         Возможно отсутствует файл либо доступ на чтение");
    return;
    }

  model_order_price = new QSqlQueryModel(this);
  model_order_price->setQuery("SELECT * FROM OrderPrice", m_db);

  model_purchase_price = new QSqlQueryModel(this);
  model_purchase_price->setQuery("SELECT * FROM PurchasePrice", m_db);

  model_cash = new QSqlQueryModel(this);
  model_cash->setQuery("SELECT * FROM Cash", m_db);

  model_users = new QSqlTableModel(this);
  model_users->setTable("USERS");
  model_users->select();

  model_goods = new QGoodsStoreModels(this, m_db);
  model_goods->select();

  ui.tableViewGood->setModel(model_goods);
  ui.tableViewGood->hideColumn(0);
  ui.tableViewGood->resizeColumnsToContents();
  ui.tableViewGood->show();
  ui.tableViewGood->setSortingEnabled(true);


  QObject::connect(ui.tableViewGood, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnDoubleClick(QModelIndex)));
  QObject::connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(OnOnConfirmSelling()));
  QObject::connect(ui.pushButtonPurchase, SIGNAL(clicked()), this, SLOT(OnOnConfirmPurchase()));
  QObject::connect(ui.pushButtonPerformCashManipulation, SIGNAL(clicked()), this, SLOT(OnOnChangeCash()));

  QObject::connect(ui.doubleSpinBoxCustomerCash, SIGNAL(valueChanged(double)), this, SLOT(UpdateChange()));
  QObject::connect(ui.lineEditOrderPrice, SIGNAL(textChanged(const QString&)), this, SLOT(UpdateChange()));
 
  model_transactions_view = new QSqlTableModelMy(this, m_db);
  model_transactions_view->setTable("TransactionsView");
  _ReloadModel(model_transactions_view);

  QObject::connect(model_transactions_view, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));

  ui.tableViewTransactions->setModel(model_transactions_view);
  ui.tableViewTransactions->hideColumn(0);
  ui.tableViewTransactions->hideColumn(7);
  ui.tableViewTransactions->hideColumn(8);
  ui.tableViewTransactions->resizeColumnsToContents();

  QStyledItemDelegate* p_deleg = dynamic_cast<QStyledItemDelegate*> (ui.tableViewTransactions->itemDelegate());

  model_transactions = new QSqlTableModel(this, m_db);;
  model_transactions->setTable("Transactions");
  _ReloadModel(model_transactions);

  _UpdateOrderPrice();
  _UpdatePurchasePrice();
  _UpdateCash();

  int num_users = model_users->rowCount();
  ui.lineEditPassword->setEchoMode(QLineEdit::Password);
  for(int i = 0; i < num_users; ++i)
    {
    QSqlRecord rec = model_users->record(i);
    if(rec.value("ROLE_ID").toInt() == 100)
      ui.comboBoxResponsible->addItem(rec.value("LOGIN").toString(), rec.value("PASSWORD").toString());
    }
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
  float good_price = rec.value(2).toFloat();
  int good_quantity = rec.value(3).toFloat();

  bool selling = ui.tabWidgetModes->currentIndex() == 0;
  bool purchasing = ui.tabWidgetModes->currentIndex() == 1;

  TransactionStatus::ETrStatus tr_status(TransactionStatus::NotUsed);

  if(selling)
    {
    if(good_quantity <= 0)
      {
      QMessageBox::warning(this, "Сообщение", "Невозможно добавить товар. Нету на складе");
      return;
      }

    tr_status =  TransactionStatus::WaitingForPayment;
    }
  else if(purchasing)
    {
    tr_status = TransactionStatus::WaitingForPurchasePayment;
    }

  if(tr_status != TransactionStatus::NotUsed)
    {
    QDateTime curr = QDateTime::currentDateTime();
    QSqlRecord rec_tr = model_transactions->record();

    QString query_str = QString("SELECT TR_ID FROM Transactions WHERE GOOD_ID = %1 AND STATUS = %2").arg(good_id).arg(tr_status);
    QSqlQuery sql_qur(m_db);
    bool qres = sql_qur.exec(query_str);

    int count = sql_qur.record().count();
    int tr_id(-1);

    if(sql_qur.next())
      tr_id = sql_qur.value(0).toInt();

    if(tr_id == -1)
      {
      rec_tr.setValue(0, QVariant());  //TR_ID
      rec_tr.setValue(1, good_id);     //GOOD_ID
      rec_tr.setValue(2, tr_status);   //STATUS
      rec_tr.setValue(3, 1);           //Number
      rec_tr.setValue(4, good_price);  //Price
      rec_tr.setValue(5, QString("")); //Comment
      rec_tr.setValue(6, curr);        //DATE
      bool res = model_transactions->insertRecord(-1, rec_tr);
      }
    else
      {
      QString query_str = QString("UPDATE Transactions SET Number=Number+1 WHERE TR_ID = %1").arg(tr_id);
      QSqlQuery sql_qur(m_db);
      bool qres = sql_qur.exec(query_str);

      _ReloadModel(model_transactions);
      _ReloadModel(model_transactions_view);
      _ReloadModel(model_goods);

    _UpdateOrderPrice();
    _UpdatePurchasePrice();

    ui.tableViewTransactions->scrollToBottom();
    }
  }

//------------------------------------------------------------
void bar::OnOnConfirmSelling()
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

  QSqlQuery sql_qur(m_db);
  bool res = sql_qur.exec(query_str);
  _ReloadModel(model_transactions);
  _ReloadModel(model_transactions_view);
  _ReloadModel(model_goods);

  ui.tableViewTransactions->scrollToBottom();
  _UpdateOrderPrice();
  ui.doubleSpinBoxCustomerCash->setValue(0);
  _UpdateCash();
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
void bar::dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
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
      if(col == Column::Number)
        {
        int new_number = topLeft.data().toInt();
        int number = std::max(0, new_number);
        update_number(number);
        }
      if(col == Column::Price)
        query_str = QString("UPDATE Transactions SET Price=%1 WHERE TR_ID=%2").arg(changed_value, 0, 'f', 2).arg(tr_id);

      if(col == Column::Sum)
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
      if(col == Column::Number)
        update_number(number);
      }

    QSqlQuery sql_qur(m_db);
    sql_qur.exec(query_str);

    int sval = ui.tableViewTransactions->verticalScrollBar()->value();
    model_transactions_view->select();
    model_transactions->select();
    ui.tableViewTransactions->verticalScrollBar()->setValue(sval);

    _UpdateOrderPrice();
    _UpdatePurchasePrice();
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

  QSqlQuery sql_qur(m_db);
  
  sql_qur.exec(query_str_update_status);
  sql_qur.exec(query_str_update_price);

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
void bar::NoGoodToolTip()
  {
  QPoint pos = QCursor::pos();
  QToolTip::showText(pos, "Товара нету на складе", 0, QRect(), 500);

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
  QString pass = ui.comboBoxResponsible->currentData().toString();
  if(ui.lineEditPassword->text() ==  pass)
    {
    double price = ui.doubleSpinBoxIncomeOutcome->value();
    QDateTime curr = QDateTime::currentDateTime();
    QSqlRecord rec_tr = model_transactions->record();

    rec_tr.setValue(0, QVariant());  //TR_ID
    rec_tr.setValue(1, 9999);     //GOOD_ID
    rec_tr.setValue(2, 3);   //STATUS
    rec_tr.setValue(3, 1);           //Number
    rec_tr.setValue(4, price);  //Price
    rec_tr.setValue(5, QString("")); //Comment
    rec_tr.setValue(6, curr);        //DATE

    int rc = model_transactions->rowCount();
    bool res = model_transactions->insertRecord(-1, rec_tr);
    
   _ReloadModel(model_transactions);
   _ReloadModel(model_transactions_view);

   Q_ASSERT(model_transactions_view->rowCount() == model_transactions->rowCount());

   _ReloadModel(model_goods);
    
   ui.lineEditPassword->clear();

    ui.tableViewTransactions->scrollToBottom();
    _UpdateCash();
    }
  else
    QMessageBox::warning(this, "Ошибка выволнения", "Неправильный пароль");
  }

void bar::_ReloadModel( QSqlTableModel* ip_model)
  {
  ip_model->select();
  while(ip_model->canFetchMore())
    {
    ip_model->fetchMore();
    }
  }
